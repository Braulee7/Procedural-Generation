#pragma once
#include <vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include "evn_device.h"
namespace evn {

	// layout of the data that will be held inside
	// each vertex of a mesh
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;

		static inline VkVertexInputBindingDescription getBindingDesc() const {
			VkVertexInputBindingDescription desc{};
			desc.binding = 0;
			desc.stride = sizeof(Vertex);
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return desc;
		}

		static inline std::array<VkVertexInputAttributeDescription, 2> getAttributes() const {
			std::array<VkVertexInputAttributeDescription, 2> attribs{};
			
			attribs[0].binding = 0;
			attribs[0].location = 0;
			attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribs[0].offset = offsetof(Vertex, pos);

			attribs[1].binding = 1;
			attribs[1].location = 1;
			attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribs[1].offset = offsetof(Vertex, color);
			return attribs;
		}
	};

	struct Data {
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
	};

	class Mesh {
	public:
		Mesh(Device& device, Data& data);
		~Mesh();
		void bind(VkCommandBuffer& command_buffer);
		void draw(VkCommandBuffer& command_buffer);

	private:
		void createBuffer(const VkDeviceSize& size,
			const VkBufferUsageFlags& usage,
			const VkMemoryPropertyFlags& props,
			VkBuffer& buffer,
			VkDeviceMemory& buffer_memory);
		void createVertexBuffer(std::vector<Vertex>& vertices);
		void createIndexBuffer(std::vector<uint32_t>& indices);
		uint32_t findMemoryType(const uint32_t type_filter, const VkMemoryPropertyFlags& properties);
		void copyBuffer();

	private:
		Device& r_device;
		VkBuffer m_vertex_buffer;
		VkDeviceMemory m_vertex_memory;
		VkBuffer m_index_buffer;
		VkDeviceMemory m_index_memory;

	};
}