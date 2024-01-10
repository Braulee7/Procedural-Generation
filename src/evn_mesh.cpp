#include "evn_mesh.h"

namespace evn{
	Mesh::Mesh(Device& device, Data& data)
		:r_device(device), m_index_count(data.indices.size()), m_vertex_count(data.vertices.size())
	{
		createVertexBuffer(data.vertices);
		createIndexBuffer(data.indices);
	}
	Mesh::~Mesh()
	{}

	void Mesh::bind(VkCommandBuffer& command_buffer)
	{
		VkBuffer buffers[] = { m_vertex_buffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, m_index_buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void Mesh::draw(VkCommandBuffer& command_buffer)
	{
		vkCmdDrawIndexed(command_buffer, m_index_count, 1, 0, 0, 0);
	}

	void Mesh::createVertexBuffer(std::vector<Vertex>& vertices)
	{
		VkDeviceSize buffer_size{ sizeof(vertices[0]) * vertices.size() };
		Buffer staging(r_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// move memory to device
		staging.map();
		staging.writeToBuffer((void*)vertices.data());

		// move staging to vertex buffer
		m_vertex_buffer = std::make_unique<Buffer>(r_device, buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_vertex_buffer->copyBuffer(staging.getBuffer(), buffer_size);
	}

	void Mesh::createIndexBuffer(std::vector<uint32_t>& indices)
	{
		VkDeviceSize buffer_size{ sizeof(indices[0]) * indices.size() };
		Buffer staging(r_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// move memory to device
		staging.map();
		staging.writeToBuffer((void*)indices.data());

		// move staging to vertex buffer
		m_index_buffer = std::make_unique<Buffer>(r_device, buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_index_buffer->copyBuffer(staging.getBuffer(), buffer_size);
	}
	
}