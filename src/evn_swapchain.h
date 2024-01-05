#pragma once
#include <array>
#include <algorithm>
#include <limits>
#include "evn_device.h"

namespace evn {
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	class Swapchain {
	public:
		Swapchain(Device& device, const VkExtent2D& extent, Window& window);
		~Swapchain();
		VkCommandBuffer beginRendering();
		void endRendering();

		inline VkRenderPass& renderPass() { return m_render_pass; }
	private: // methods
		void init();
		void createSwapchain();
		void createImageViews();
		void createRenderPass();
		void createFrameBuffer();
		void createSyncObjects();
		void cleanUpSwapchain();
		void createCommandBuffers();
		void freeCommandBuffers();
		void createDepthResources();

		// resize the swapchain
		void recreateSwapchain();
		static void frameBufferResizeCallback(GLFWwindow*, int, int);
		// rendering methods
		void beginRenderPass(VkCommandBuffer& command_buffer);
		void endRenderPass(VkCommandBuffer& command_buffer);
		void submitCommands(VkCommandBuffer& command_buffer);
		// helper methods
		VkSurfaceFormatKHR chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);
		VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat findDepthFormat();
	private:
		// references
		Device &r_device;
		Window& r_window;
		// swapchain variables
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> sc_images;
		std::vector<VkImageView> sc_image_views;
		VkFormat sc_format;
		VkExtent2D sc_extent;
		VkExtent2D m_window_extent;
		std::vector<VkFramebuffer> sc_framebuffers;
		// depth resources
		std::vector<VkImage> m_depth_images;
		std::vector<VkDeviceMemory> m_depth_image_memories;
		std::vector<VkImageView> m_depth_image_views;
		VkFormat sc_depth_format;
		bool m_resized;
		// synchronisation variables
		std::vector<VkSemaphore> m_images_available;
		std::vector<VkSemaphore> m_renders_finished;
		std::vector<VkFence> m_in_flight_fences;
		uint32_t m_curr_frame;
		// rendering variables
		VkRenderPass m_render_pass;
		std::vector<VkCommandBuffer> m_command_buffers;
		uint32_t m_image_index;
	};
}