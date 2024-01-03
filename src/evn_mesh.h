#pragma once
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "evn_device.h"
#include "evn_buffer.h"
namespace evn {

	// layout of the data that will be held inside
	// each vertex of a mesh
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;

		static inline VkVertexInputBindingDescription getBindingDesc() {
			VkVertexInputBindingDescription desc{};
			desc.binding = 0;
			desc.stride = sizeof(Vertex);
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return desc;
		}

		static inline std::array<VkVertexInputAttributeDescription, 2> getAttributes() {
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
		
		void createVertexBuffer(std::vector<Vertex>& vertices);
		void createIndexBuffer(std::vector<uint32_t>& indices);
		

	private:
		Device& r_device;
		std::unique_ptr<Buffer> m_vertex_buffer;
		std::unique_ptr<Buffer> m_index_buffer;
		uint32_t m_index_count;
		uint32_t m_vertex_count;

	};
}