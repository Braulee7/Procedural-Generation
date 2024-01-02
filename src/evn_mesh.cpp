#include "evn_mesh.h"

namespace evn{
	Mesh::Mesh(Device& device, Data& data)
		:r_device(device)
	{
		createVertexBuffer(data.vertices);
		createIndexBuffer(data.indices);
	}
	Mesh::~Mesh()
	{
		vkDestroyBuffer(r_device.device(), m_index_buffer, nullptr);
		vkFreeMemory(r_device.device(), m_index_memory, nullptr);
		
		vkDestroyBuffer(r_device.device(), m_vertex_buffer, nullptr);
		vkFreeMemory(r_device.device(), m_vertex_memory, nullptr);

	}
	void Mesh::createBuffer(const VkDeviceSize& size,
		const VkBufferUsageFlags& usage,
		const VkMemoryPropertyFlags& props,
		VkBuffer& buffer,
		VkDeviceMemory& buffer_memory)
	{
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(r_device.device(), &buffer_info, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a buffer");

		VkMemoryRequirements memory_reqs;
		vkGetBufferMemoryRequirements(r_device.device(), buffer, &memory_reqs);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = memory_reqs.size;
		alloc_info.memoryTypeIndex = r_device.findMemoryTypes(memory_reqs.memoryTypeBits, props);

		if (vkAllocateMemory(r_device.device(), nullptr, &buffer_memory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate buffer memory");

		vkBindBufferMemory(r_device.device(), buffer, buffer_memory, 0); 
	}
	
}