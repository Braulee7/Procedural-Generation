#pragma once
#include <algorithm>
#include <limits>
#include "evn_device.h"

namespace evn {
	const int MAX_FRAMES_IN_FLIGHT = 2;

	class Swapchain {
	public:
		Swapchain(Device& device, const VkExtent2D& extent);
		~Swapchain();

	private: // methods
		void init();
		void createSwapchain();
		void createImageViews();
		void createRenderPass();
		void createFrameBuffer();
		void createSyncObjects();
		void cleanUpSwapchain();

		// helper methods
		VkSurfaceFormatKHR chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);
		VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	private:
		Device &r_device;
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> sc_images;
		std::vector<VkImageView> sc_image_views;
		VkFormat sc_format;
		VkExtent2D sc_extent;
		VkExtent2D m_window_extent;
		std::vector<VkFramebuffer> sc_framebuffers;
		VkRenderPass m_render_pass;
		// synchronisation variables
		std::vector<VkSemaphore> m_images_available;
		std::vector<VkSemaphore> m_renders_finished;
		std::vector<VkFence> m_in_flight_fences;
		uint32_t m_curr_frame;
	};
}