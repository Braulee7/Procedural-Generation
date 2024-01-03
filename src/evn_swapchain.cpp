#include "evn_swapchain.h"

namespace evn {
	Swapchain::Swapchain(Device& device, const VkExtent2D& extent)
		: r_device(device), m_window_extent(extent)
	{
		init();
	}

	Swapchain::~Swapchain()
	{
		cleanUpSwapchain();

		vkDestroyRenderPass(r_device.device(), m_render_pass, nullptr);
		for (int i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(r_device.device(), m_images_available[i], nullptr);
			vkDestroySemaphore(r_device.device(), m_renders_finished[i], nullptr);
			vkDestroyFence	  (r_device.device(), m_in_flight_fences[i], nullptr);
		}
	}

	void Swapchain::init()
	{
		createSwapchain();
		createImageViews();
		createRenderPass();
		createFrameBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	void Swapchain::createSwapchain()
	{
		SwapchainSupportDetails details{ r_device.swapchainSupport() };
		uint32_t image_count{ 0 };
		// get the details
		VkSurfaceFormatKHR surface_format{ chooseSwapchainFormat(details.formats)};
		VkPresentModeKHR present_mode{ chooseSwapchainPresentMode(details.present_modes) };
		VkExtent2D extent{ chooseSwapchainExtent(details.capabilities) };

		// save the values
		sc_format = surface_format.format;
		sc_extent = extent;
		
		image_count = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 &&
			image_count > details.capabilities.maxImageCount)
			image_count = details.capabilities.maxImageCount;

		// fill in the create struct
		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = r_device.surface();
		create_info.minImageCount = image_count;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageFormat = surface_format.format;
		create_info.imageArrayLayers = 1;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.imageExtent = extent;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.preTransform = details.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		QueueFamilyIndices indices{ r_device.getQueueFamilies() };
		uint32_t queue_family_indices[] = { indices.graphics_family.value(),
											indices.present_family.value() };
		
		if (indices.graphics_family != indices.present_family) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0; // Optional
			create_info.pQueueFamilyIndices = nullptr; // Optional
		}

		if (vkCreateSwapchainKHR(r_device.device(), &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain");

		// get swapchain images
		vkGetSwapchainImagesKHR(r_device.device(), m_swapchain, &image_count, nullptr);
		sc_images.resize(image_count);
		vkGetSwapchainImagesKHR(r_device.device(), m_swapchain, &image_count, sc_images.data());
	}

	void Swapchain::createImageViews()
	{
		sc_image_views.resize(sc_images.size());

		for (int i{ 0 }; i < sc_image_views.size(); i++) {
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = sc_images[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = sc_format;
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(r_device.device(), &create_info, nullptr, &sc_image_views[i])
				!= VK_SUCCESS)
				throw std::runtime_error("Failed to create image view");
		}
	}

	void Swapchain::createRenderPass()
	{
		VkAttachmentDescription color_attachment{};
		color_attachment.format = sc_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(r_device.device(), &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass");
	}

	void Swapchain::createFrameBuffer()
	{
		sc_framebuffers.resize(sc_image_views.size());

		for (size_t i{ 0 }; i < sc_image_views.size(); i++) {
			VkImageView attachments[] = { sc_image_views[i] };

			VkFramebufferCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = m_render_pass;
			create_info.attachmentCount = 1;
			create_info.pAttachments = attachments;
			create_info.width = sc_extent.width;
			create_info.height = sc_extent.height;
			create_info.layers = 1;

			if (vkCreateFramebuffer(r_device.device(), &create_info, nullptr, &sc_framebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create framebuffer");
		}
	}

	void Swapchain::createSyncObjects()
	{
		m_images_available.resize(MAX_FRAMES_IN_FLIGHT);
		m_renders_finished.resize(MAX_FRAMES_IN_FLIGHT);
		m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(r_device.device(), &semaphore_info, nullptr, &m_images_available[i]) != VK_SUCCESS ||
				vkCreateSemaphore(r_device.device(), &semaphore_info, nullptr, &m_renders_finished[i]) != VK_SUCCESS ||
				vkCreateFence(r_device.device(), &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create sync objects");
		}
	}

	void Swapchain::createCommandBuffers()
	{
		m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = r_device.commandPool();
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

		if (vkAllocateCommandBuffers(r_device.device(), &info, m_command_buffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to create command buffers");
	}

	void Swapchain::cleanUpSwapchain()
	{
		for (size_t i{ 0 }; i < sc_framebuffers.size(); i++) {
			vkDestroyFramebuffer(r_device.device(), sc_framebuffers[i], nullptr);
		}

		for (size_t i{ 0 }; i < sc_image_views.size(); i++) {
			vkDestroyImageView(r_device.device(), sc_image_views[i], nullptr);
		}

		vkDestroySwapchainKHR(r_device.device(), m_swapchain, nullptr);
	}

	VkSurfaceFormatKHR Swapchain::chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
				return format;
		}

		// if we don't get what we're looking for the first fomat should
		// be enough
		return formats[0];
	}

	VkPresentModeKHR Swapchain::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& modes)
	{
		for (const auto& mode : modes) {
			// if possible we would like tripple buffering
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}
		// this mode is typically supported and reliable
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
			return capabilities.currentExtent;
		}

		VkExtent2D actual{ m_window_extent };

		actual.width = std::clamp(actual.width, 
			capabilities.minImageExtent.width, 
			capabilities.maxImageExtent.width);
		actual.height = std::clamp(actual.height,
			capabilities.minImageExtent.height, 
			capabilities.maxImageExtent.height);

		return actual;
	}


}

