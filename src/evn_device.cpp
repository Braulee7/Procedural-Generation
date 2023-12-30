#include "evn_device.h"

namespace evn {

	// debug local methods
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	Device::Device(Window& window)
		: m_physical_device(VK_NULL_HANDLE), r_window(window)
	{
		createInstance();
		if (debug)
			setUpDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	Device::~Device()
	{
		vkDestroyDevice(m_device, nullptr);
		if (debug)
			DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void Device::createInstance()
	{
		// debug
		if (debug && !checkValidationSupport())
			throw std::runtime_error("Validation layers requested but none are availabel");

		// basic application info
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "EVN Engine";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "No Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		// create struct
		VkInstanceCreateInfo create_info{};
		create_info.pApplicationInfo = &app_info;
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		// get the required glfw3 extensions
		auto extensions = getExtensions();
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		// debug info
		// created before debug so lifetime extends beyond
		VkDebugUtilsMessengerCreateInfoEXT debug_info{};
		if (debug) {
			// add layers
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
			// add the debug info so we are notified during 
			// creation and destruction of instance
			populateDebugMessengerCreateInfo(debug_info);
			create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info;
		}
		else
			create_info.enabledLayerCount = 0;

		if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan instance");
	}

	void Device::createSurface()
	{
		// glfw cross platform surface
		if (glfwCreateWindowSurface(m_instance, r_window.getWindow(), nullptr, &m_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the window surface");
	}

	void Device::pickPhysicalDevice()
	{
		uint32_t device_count{ 0 };
		std::vector<VkPhysicalDevice> devices;

		// 2 step process to get devices
		vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
		if (!device_count) throw std::runtime_error("No devices capable of running vulkan");
		devices.resize(device_count);
		vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

		// get the first device that fits our needs
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				m_physical_device = device;
				break;
			}
		}

		if (m_physical_device == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable device");
	}

	void Device::createLogicalDevice()
	{
		QueueFamilyIndices indices{ findQueueFamilies(m_physical_device) };
		// explicitly treat both families as if their different
		std::vector<VkDeviceQueueCreateInfo> create_info{};
		std::set<uint32_t> unique_families =
		{ indices.graphics_family.value(), indices.present_family.value() };

		// add a new create info struct for each of the unique
		// families if they're different
		const float queue_priority{ 1.0 };
		for (auto& queue_family : unique_families) {
			VkDeviceQueueCreateInfo device_info{};

			device_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_info.queueFamilyIndex = queue_family;
			device_info.queueCount = 1;
			device_info.pQueuePriorities = &queue_priority;
			create_info.push_back(device_info);
		}

		// specify which features we will be using
		VkPhysicalDeviceFeatures feats{ VK_FALSE };

		// create logical device
		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pQueueCreateInfos = create_info.data();
		info.queueCreateInfoCount = static_cast<uint32_t>(create_info.size());
		info.pEnabledFeatures = &feats;

		// extensions
		info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		info.ppEnabledExtensionNames = device_extensions.data();

		// debug layers
		if (debug) {
			info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			info.ppEnabledLayerNames = validation_layers.data();
		}
		else
			info.enabledLayerCount = 0;

		// create the logical device
		if (vkCreateDevice(m_physical_device, &info, nullptr, &m_device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");
		// get the queues
		vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
		vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);
	}

	std::vector<const char*> Device::getExtensions() const
	{
		uint32_t extension_count{ 0 };
		const char** extensions{ nullptr };

		extensions = glfwGetRequiredInstanceExtensions(&extension_count);

		std::vector<const char*>extensions_v(extensions, extensions + extension_count);
		
		// debug layers
		if (debug)
			extensions_v.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions_v;
	}

	bool Device::isDeviceSuitable(const VkPhysicalDevice& device) const
	{
		QueueFamilyIndices indices{ findQueueFamilies(device) };
		bool extension_supported{ checkDeviceExtensionSupport(device) };
		bool swap_chain_adequate{ false };

		if (extension_supported) {
			SwapchainSupportDetails details{ querySwapchainSupport(device) };
			swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
		}

		return indices.isComplete() && extension_supported && swap_chain_adequate;
	}

	bool Device::checkDeviceExtensionSupport(const VkPhysicalDevice& device) const
	{
		uint32_t extension_count{ 0 };
		std::vector<VkExtensionProperties> props;
		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
		// get properties
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
		props.resize(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, props.data());

		// make sure all required extensions are on device
		for (const auto& extension : props)
			required_extensions.erase(extension.extensionName);

		return required_extensions.empty();
	}

	QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice& device) const
	{
		QueueFamilyIndices indices{};
		uint32_t queue_fam_count{ 0 };
		std::vector<VkQueueFamilyProperties> family_props;

		// get the properties
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_fam_count, nullptr);
		family_props.resize(queue_fam_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_fam_count, family_props.data());

		// only need one family that supports the VK_QUEUE_GRAPHICS_BIT
		int i{ 0 };
		for (const auto& queue : family_props) {
			VkBool32 present{ false };
			// check if device supports the grahpics queue
			if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics_family = i;
			// check if it can present on a surface
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present);
			if (present)
				indices.present_family = i;
			i++;
		}

		return indices;
	}

	SwapchainSupportDetails Device::querySwapchainSupport(const VkPhysicalDevice& device) const
	{
		SwapchainSupportDetails detail{};
		uint32_t format_count{ 0 };
		uint32_t present_mode_count{ 0 };

		// fill in the struct
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &detail.capabilities);
		// get formats
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
		if (format_count != 0) {
			detail.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, detail.formats.data());
		}
		// present modes
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);
		if (present_mode_count != 0) {
			detail.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, detail.present_modes.data());
		}

		return detail;
	}

	void Device::setUpDebugMessenger()
	{
		if (!debug) return;

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		populateDebugMessengerCreateInfo(create_info);

		if (CreateDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
			throw std::runtime_error("failed to set up debug messenger");
	}

	void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info)
	{
		create_info = {};
		create_info.sType = // the type of messenger
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = // the severity we want to be notified for
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = // message types, all enabled for now
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = // the callback function we are using
			debugCallback;
		create_info.pUserData // can be the application class but is optional
			= nullptr; // Optional
	}
	bool Device::checkValidationSupport()
	{
		uint32_t layer_count{ 0 };
		std::vector<VkLayerProperties> available_layers;
		// get the layers
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		available_layers.resize(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		for (const auto layer : validation_layers) {
			bool layer_found{ false };
			for (const auto& layer_prop : available_layers) {
				if (strcmp(layer, layer_prop.layerName) == 0) {
					layer_found = true;
					break;
				}
			}
			// if one layer is missing return false don't
			// continue searching
			if (!layer_found) return false;
		}

		// all layers found
		return true;
	}
}

