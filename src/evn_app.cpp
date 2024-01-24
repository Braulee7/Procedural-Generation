#include <chrono>
#include "evn_app.h"
#include "evn_terrain.h"

const std::vector<evn::Vertex> vertices = {
	{{-0.5f,  0.0f,  0.5f},    {1.0, 0.0,0.0}},    // index 0  front left corner
	{{ 0.5f,  0.0f, -0.5f},    {0.0, 0.0,1.0}},    // index 1  back right corner
	{{ 0.5f,  0.0f,  0.5f},    {1.0, 0.0,0.0}},    // index 2  front right corner
	{{-0.5f,  0.0f, -0.5f},    {0.0, 0.0,1.0}},    // index 3  back left corner
	{{ 0.0f,  0.7f,  0.0f},    {0.0, 0.0,1.0}},	   // index 4  top of pyramid
};

const std::vector<uint32_t> indices = {
	0, 1, 3,    //bottom of pyramid, left half of square
	0, 2, 1,    //right half of square
	0, 3, 4,    //left triangle
	3, 1, 4,    //back triangle
	1, 2, 4,    //right triangle
	2, 0, 4,
};

namespace evn {
	App::App(const std::string& name)
		: m_name(name), m_window(Window(WIDTH, HEIGHT, m_name)),
		m_device(m_window), m_swapchain(m_device, m_window.getExtent(), m_window),
		m_cam(m_device, WIDTH, HEIGHT, 3.0f)
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
		auto start{ std::chrono::steady_clock::now() };
		float delta_time{ 0 };
		// TEMPORARY CREATE AN OBJECT
		Data data{ vertices, indices };
		Mesh obj(m_device, data);

		Terrain terrain(m_device);
		while (!m_window.shouldClose()) {

			glfwPollEvents();
			
			auto command_buffer = m_swapchain.beginRendering();
			m_pipeline->bind(command_buffer);
			m_cam.update(command_buffer, m_layout, m_swapchain.currentFrame(),
				m_window.getWindow(), delta_time);
			// obj.bind(command_buffer);
			// obj.draw(command_buffer);
			terrain.update(command_buffer);

			m_swapchain.endRendering();

			auto end{ std::chrono::steady_clock::now() };
			delta_time = std::chrono::duration_cast<std::chrono::microseconds>
				(end - start).count() / 1000000.0f;
			start = end;
			
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
		pipeline_info.setLayoutCount = 1;
		pipeline_info.pSetLayouts = &m_cam.layout();

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