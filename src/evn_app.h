#pragma once
#include "evn_swapchain.h"
#include "evn_device.h"
#include "evn_pipeline.h"
#include "evn_camera.h"

namespace evn {
	class App {
	public:
		App(const std::string& name);
		~App();
		void run();
	private:// methods
		void setUpPipelineLayout();
		void createPipeline();
	private:
		const uint32_t WIDTH = 1280;
		const uint32_t HEIGHT = 720;
		std::string m_name;
		Window m_window;
		Device m_device;
		Swapchain m_swapchain;
		Camera m_cam;
		VkPipelineLayout m_layout;
		std::unique_ptr<Pipeline> m_pipeline;
		
	};
}