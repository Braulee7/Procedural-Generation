#include "evn_camera.h"

namespace evn {
	Camera::Camera(Device& device, uint32_t width, uint32_t height,
		float speed, float sens)
		: r_device(device), m_pos(0, 0, -3), m_front(0, 0, 1), m_up(0, 1, 0),
		m_direction(0), m_view(0), m_yaw(-90.0f), 
		m_pitch(0), m_first_click(true), m_last_x(height / 2),
		m_last_y(width / 2), m_sens(sens), m_speed(speed),
		m_width(width), m_height(height)
	{
		createUniformBuffers();
		createUniformBuffers();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
	}

	Camera::~Camera()
	{

		vkDestroyDescriptorPool(r_device.device(), m_pool, nullptr);
		vkDestroyDescriptorSetLayout(r_device.device(), m_descriptor_layout, nullptr);
		
	}

	void Camera::update(VkCommandBuffer& command_buffer, VkPipelineLayout& pipeline_layout,
		uint32_t image_index, GLFWwindow* window, float delta_time)
	{
		// process input from user
		processInput(window, delta_time);
		// TEMPORARY
		ViewUniformBuffer ubo{};
		m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
		ubo.view = m_view;
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)(m_width / m_height),
			0.1f, 150.0f);
		ubo.proj[1][1] *= -1;
		m_uniform_buffers[image_index]->writeToBuffer((void*)&ubo);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout, 0, 1, &m_descriptor_sets[image_index], 0, nullptr);
	}

	void Camera::createUniformBuffers()
	{
		m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (int i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
			m_uniform_buffers[i] = std::make_unique<Buffer>(r_device, sizeof(ViewUniformBuffer),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			m_uniform_buffers[i]->map();
		}
	}

	void Camera::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding ubo_binding{};
		ubo_binding.binding = 0;
		ubo_binding.descriptorCount = 1;
		ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ubo_binding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &ubo_binding;

		if (vkCreateDescriptorSetLayout(r_device.device(), &layout_info, nullptr, &m_descriptor_layout)
			!= VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout");
	}

	void Camera::createDescriptorPool()
	{
		VkDescriptorPoolSize pool_size{};
		pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pPoolSizes = &pool_size;
		create_info.poolSizeCount = 1;
		create_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(r_device.device(), &create_info, nullptr, &m_pool)
			!= VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool");

	}

	void Camera::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptor_layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = m_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		alloc_info.pSetLayouts = layouts.data();

		m_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(r_device.device(), &alloc_info, m_descriptor_sets.data())
			!= VK_SUCCESS) {
			std::cout << "Failed to allocate descriptor sets\n";
			throw std::runtime_error("Failed to allocate descriptor sets");
		}

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = m_uniform_buffers[i]->getBuffer();
			buffer_info.offset = 0;
			buffer_info.range  = VK_WHOLE_SIZE;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_descriptor_sets[i];
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &buffer_info;

			vkUpdateDescriptorSets(r_device.device(), 1, &write, 0, nullptr);
		}

	}

	void Camera::processInput(GLFWwindow* window, float delta_time)
	{
		/* Keyboard inputs for movement */
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {//forward
			m_pos += (m_speed * delta_time) * m_front;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {//left
			m_pos -= glm::normalize(glm::cross(m_front, m_up)) * (m_speed * delta_time);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {//backward
			m_pos -= (m_speed * delta_time) * m_front;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {//right
			m_pos += glm::normalize(glm::cross(m_front, m_up)) * (m_speed * delta_time);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){// up
			m_pos += glm::vec3(0.0, m_speed * delta_time, 0.0);
		}

		/* Mouse inputs for looking around */
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double x = 0, y = 0;//current position of the mouse
			glfwGetCursorPos(window, &x, &y);

			//hide the cursor so it doesn't go off screen
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			//handle the direction of teh mouse
			processMouse(window, x, y);
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_first_click = true;
		}
	}

	void Camera::processMouse(GLFWwindow* window, double x, double y)
	{
		//getting the angles
		if (m_first_click) {
			m_last_x = x;
			m_last_y = y;
			m_first_click = false;
		}

		double xOffset = x - m_last_x;
		double yOffset = y - m_last_y;
		m_last_x = x;
		m_last_y = y;

		yOffset *= m_sens;
		xOffset *= m_sens;

		//set the angle
		m_yaw += xOffset;
		m_pitch += yOffset;

		//limit the angles to right angles
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_yaw < -89.0f)
			m_yaw = 89.0f;

		//calculate the acutal direction using trigonmetry in the third dimension
		m_direction.x = (float)cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_direction.y = (float)sin(glm::radians(m_pitch));
		m_direction.z = (float)sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = m_direction =  glm::normalize(m_direction);
	}


}