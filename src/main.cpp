#include "evn_window.h"
#include "evn_device.h"
#include "evn_swapchain.h"
#include "evn_pipeline.h"

// vertices
const std::vector<evn::Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}	}, 
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
};
// indices
const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0
};


int main(void)
{
	// testing window
	evn::Window window(1280, 720, "evolution engine");
	evn::Device device(window);
	evn::Swapchain swapchain(device, window.getExtent(), window);
	// pipeline
	evn::PipelineConfigInfo config{};

	VkPipelineLayoutCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_info.setLayoutCount = 0;
	//pipeline_info.pSetLayouts = &descriptor_set_layout;
	pipeline_info.pushConstantRangeCount = 0;
	pipeline_info.pPushConstantRanges = nullptr;
	VkPipelineLayout pipeline_layout;

	if (vkCreatePipelineLayout(device.device(), &pipeline_info, nullptr, &pipeline_layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");
	evn::Pipeline::defaultPipelineConfigInfo(config);
	config.pipeline_layout = pipeline_layout;
	config.render_pass = swapchain.renderPass();

	evn::Pipeline pipeline(device, "shaders/vert.spv", "shaders/frag.spv", config);

	evn::Data data {vertices, indices};
	evn::Mesh mesh(device, data);

	while (!window.shouldClose()) {
		glfwPollEvents();
		auto command_buffer = swapchain.beginRendering();
		pipeline.bind(command_buffer);
		mesh.bind(command_buffer);
		mesh.draw(command_buffer);
		swapchain.endRendering();
	}

	vkDeviceWaitIdle(device.device());

	vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
	return 0;
}