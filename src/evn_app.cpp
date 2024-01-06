#include <chrono>
#include "evn_app.h"

namespace evn {
	App::App(const std::string& name)
		: m_name(name), m_window(Window(WIDTH, HEIGHT, m_name)),
		m_device(m_window), m_swapchain(m_device, m_window.getExtent(), m_window)
	{
		setUpPipelineLayout();
		createPipeline();
	}

	App::~App()
	{
		vkDestroyPipelineLayout(m_device.device(), m_layout, nullptr);
	}
	void App::run()
	{

		while (!m_window.shouldClose()) {
			glfwPollEvents();
			
			auto command_buffer = m_swapchain.beginRendering();
			m_pipeline->bind(command_buffer);
			
			m_swapchain.endRendering();
		}

		vkDeviceWaitIdle(m_device.device());
	}
	void App::setUpPipelineLayout()
	{

		VkPipelineLayoutCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_info.setLayoutCount = 0;
		pipeline_info.pushConstantRangeCount = 0;
		pipeline_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_device.device(), &pipeline_info, nullptr, &m_layout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}
	void App::createPipeline()
	{
		PipelineConfigInfo config{};
		Pipeline::defaultPipelineConfigInfo(config);
		config.pipeline_layout = m_layout;
		config.render_pass = m_swapchain.renderPass();

		m_pipeline = std::make_unique<Pipeline>(m_device,
			"shaders/shader.vert.spv", "shaders/shader.frag.spv", config);
	}
}