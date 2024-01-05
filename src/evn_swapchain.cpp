#include "evn_swapchain.h"

namespace evn {
	Swapchain::Swapchain(Device& device, const VkExtent2D& extent, Window& window)
		: r_device(device), r_window(window), m_window_extent(extent), m_resized(false),
		m_curr_frame(0), m_image_index(0)
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

	VkCommandBuffer Swapchain::beginRendering()
	{
		vkWaitForFences(r_device.device(), 1, &m_in_flight_fences[m_curr_frame],
			VK_TRUE, UINT64_MAX);

		VkResult result{ vkAcquireNextImageKHR(r_device.device(), m_swapchain, UINT64_MAX,
			m_images_available[m_curr_frame], VK_NULL_HANDLE, &m_image_index) };

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return nullptr;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swapchain image");

		vkResetFences(r_device.device(), 1, &m_in_flight_fences[m_curr_frame]);

		// begin recording
		vkResetCommandBuffer(m_command_buffers[m_curr_frame], 0);
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(m_command_buffers[m_curr_frame], &begin_info) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer");

		beginRenderPass(m_command_buffers[m_curr_frame]);

		return m_command_buffers[m_curr_frame];
	}

	void Swapchain::endRendering()
	{
		endRenderPass(m_command_buffers[m_curr_frame]);
		vkEndCommandBuffer(m_command_buffers[m_curr_frame]);
		submitCommands(m_command_buffers[m_curr_frame]);
	}

	void Swapchain::init()
	{
		// set the window resizing methods to be here
		glfwSetWindowUserPointer(r_window.getWindow(), this);
		glfwSetFramebufferSizeCallback(r_window.getWindow(), frameBufferResizeCallback);
		createSwapchain();
		createImageViews();
		createRenderPass();
		createDepthResources();
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
		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = findDepthFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
		VkRenderPassCreateInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
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
			VkImageView attachments[] = { sc_image_views[i], m_depth_image_views[i]};

			VkFramebufferCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = m_render_pass;
			create_info.attachmentCount = 2;
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

	void Swapchain::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			r_device.device(),
			r_device.commandPool(),
			static_cast<uint32_t>(m_command_buffers.size()),
			m_command_buffers.data());
		m_command_buffers.clear();
	}

	void Swapchain::createDepthResources()
	{
		VkFormat depth_format{ findDepthFormat() };
		sc_depth_format = depth_format;
		size_t image_count{ sc_images.size() };

		m_depth_images.resize(image_count);
		m_depth_image_memories.resize(image_count);
		m_depth_image_views.resize(image_count);

		for (int i{ 0 }; i < image_count; i++) {
			VkImageCreateInfo image_info{};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.extent.width = sc_extent.width;
			image_info.extent.height = sc_extent.height;
			image_info.extent.depth = 1;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = depth_format;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.flags = 0;

			r_device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_depth_images[i], m_depth_image_memories[i]);

			VkImageViewCreateInfo view_info{};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = m_depth_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = depth_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(r_device.device(), &view_info, nullptr, &m_depth_image_views[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create depth image views");
		}
	}

	void Swapchain::cleanUpSwapchain()
	{
		for (size_t i{ 0 }; i < sc_framebuffers.size(); i++) {
			vkDestroyFramebuffer(r_device.device(), sc_framebuffers[i], nullptr);
		}

		for (size_t i{ 0 }; i < sc_image_views.size(); i++) {
			vkDestroyImageView(r_device.device(), sc_image_views[i], nullptr);
		}

		for (size_t i{ 0 }; i < m_depth_images.size(); i++) {
			vkDestroyImageView(r_device.device(), m_depth_image_views[i], nullptr);
			vkDestroyImage(r_device.device(), m_depth_images[i], nullptr);
			vkFreeMemory(r_device.device(), m_depth_image_memories[i], nullptr);
		}

		vkDestroySwapchainKHR(r_device.device(), m_swapchain, nullptr);
	}

	void Swapchain::recreateSwapchain()
	{
		int width{ 0 }, height{ 0 };
		glfwGetFramebufferSize(r_window.getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(r_window.getWindow(), &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(r_device.device());

		cleanUpSwapchain();

		createSwapchain();
		createImageViews();
		createDepthResources();
		createFrameBuffer();
	}

	void Swapchain::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Swapchain*>(glfwGetWindowUserPointer(window));
		app->m_resized = true;
	}

	void Swapchain::beginRenderPass(VkCommandBuffer& command_buffer)
	{
		VkRenderPassBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = m_render_pass;
		begin_info.framebuffer = sc_framebuffers[m_image_index];
		begin_info.renderArea.offset = { 0, 0 };
		begin_info.renderArea.extent = sc_extent;

		std::array<VkClearValue, 2> clear_vals{};
		clear_vals[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clear_vals[1].depthStencil = { 1.0f, 0 };
		begin_info.clearValueCount = static_cast<uint32_t>(clear_vals.size());
		begin_info.pClearValues = clear_vals.data();

		vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(sc_extent.width);
		viewport.height = static_cast<float>(sc_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = sc_extent;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void Swapchain::endRenderPass(VkCommandBuffer& command_buffer)
	{
		vkCmdEndRenderPass(command_buffer);
	}

	void Swapchain::submitCommands(VkCommandBuffer& command_buffer)
	{
		VkSubmitInfo submit_info{};
		VkSemaphore wait_semaphores[] = { m_images_available[m_curr_frame] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signal_semaphores[] = { m_renders_finished[m_curr_frame] };

		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		if (vkQueueSubmit(r_device.graphicsQueue(), 1, &submit_info,
			m_in_flight_fences[m_curr_frame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command to buffer");

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = { m_swapchain };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;
		present_info.pImageIndices = &m_image_index;
		present_info.pResults = nullptr;

		auto result = vkQueuePresentKHR(r_device.presentQueue(), &present_info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR || m_resized)
		{
			m_resized = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to present image");

		m_curr_frame = (m_curr_frame + 1) % MAX_FRAMES_IN_FLIGHT;

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

	VkFormat Swapchain::findDepthFormat()
	{
		return r_device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}


}

