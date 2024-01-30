#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "evn_buffer.h"
#include "evn_swapchain.h"

namespace evn {
	struct ViewUniformBuffer {
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Camera {
	public:
		Camera(Device& device, uint32_t width, uint32_t height,
				float speed=1.5f, float sens=0.05f);
		~Camera();
		void update(VkCommandBuffer& command_buffer, VkPipelineLayout& pipeline_layout, 
			uint32_t curr_frame, GLFWwindow* window, float delta_time);
		inline VkDescriptorSetLayout& layout() { return m_descriptor_layout; }

	public:
		glm::vec3 m_pos; // public to allow other classes to get access
	private:
		// uniform buffer methods
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();
		// input methods
		void processInput(GLFWwindow* window, float delta_time);
		void processMouse(GLFWwindow* window, double x, double y);
	private:
		// uniform buffer variables
		Device& r_device;
		std::vector<std::unique_ptr<Buffer>> m_uniform_buffers;
		VkDescriptorSetLayout m_descriptor_layout;
		VkDescriptorPool m_pool;
		std::vector<VkDescriptorSet> m_descriptor_sets;

		// look at vectors
		// position
		glm::vec3 m_front;
		glm::vec3 m_up;
		glm::vec3 m_direction;
		glm::mat4 m_view;

		// angle variables
		double m_yaw;
		double m_pitch;

		// input variables
		bool   m_first_click;
		double m_last_x;
		double m_last_y;

		// speed 
		float m_sens;
		float m_speed;

		// screen dimensions
		uint32_t m_width;
		uint32_t m_height;
	};
}