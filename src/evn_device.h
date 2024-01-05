#pragma once
// vulkan
#include "evn_window.h"
// std
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <set>
#include <optional>
namespace evn {

	// structs to help get the queue families
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		inline bool isComplete() const {
			return graphics_family.has_value() && present_family.has_value();
		}
	};
	// helper struct to hold all information needed
	// for the swapchain
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	class Device {
	public:
		// constructors, must have a window created before
		// creating a device
		Device(Window& window);
		~Device();
		// delete all other constructors we don't want copies
		// or default
		Device() = delete;
		Device(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		Device&& operator=(Device&&) = delete;
		// getter methods
		inline SwapchainSupportDetails swapchainSupport() const
		{
			return querySwapchainSupport(m_physical_device);
		}
		inline VkQueue& graphicsQueue() { return m_graphics_queue; };
		inline VkQueue& presentQueue() { return m_present_queue; };
		inline VkSurfaceKHR& surface() { return m_surface; }
		inline VkDevice& device() { return m_device; }
		inline VkCommandPool& commandPool() { return m_command_pool; }
		QueueFamilyIndices getQueueFamilies() const;
		// helper methods
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer& command_buffer);
		uint32_t findMemoryType(const uint32_t type_filter, VkMemoryPropertyFlags props);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& canidates, const VkImageTiling& tiling,
			const VkFormatFeatureFlags& features);
		void createImageWithInfo(
			const VkImageCreateInfo& image_info,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& image_memory);
	private: // methods
		// creating
		void createInstance();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		// helper
		std::vector<const char*> getExtensions() const;
		bool isDeviceSuitable(const VkPhysicalDevice& device) const;
		bool checkDeviceExtensionSupport(const VkPhysicalDevice& device) const;
		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device) const;
		SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice& device) const;
		// debug methods
		void setUpDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);
		bool checkValidationSupport();
		
		void createCommandPool();
	private:
		VkInstance m_instance;
		VkPhysicalDevice m_physical_device;
		VkDevice m_device;
		VkQueue m_graphics_queue;
		VkQueue m_present_queue;
		VkSurfaceKHR m_surface;
		VkCommandPool m_command_pool;
		Window& r_window;
		
		// debug
		VkDebugUtilsMessengerEXT m_debug_messenger;
#ifdef NDEBUG
		const bool debug = false;
#else
		const bool debug = true;
#endif // NDEBUG


		// constants
		const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
} // evn