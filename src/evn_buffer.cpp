#include "evn_buffer.h"

namespace evn {
	Buffer::Buffer(Device& device, const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties)
		: r_device(device), m_buffer(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE), m_size(size), p_data(nullptr)
	{
		createBuffer(size, usage, properties, m_buffer, m_memory);
	}

	Buffer::~Buffer()
	{
		if (p_data) {
			vkUnmapMemory(r_device.device(), m_memory);
			p_data = nullptr;
		}
		vkDestroyBuffer(r_device.device(), m_buffer, nullptr);
		vkFreeMemory(r_device.device(), m_memory, nullptr);
	}

	void Buffer::copyBuffer(VkBuffer& src_buffer, const VkDeviceSize& size)
	{
		auto command_buffer{ r_device.beginSingleTimeCommands() };

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;

		vkCmdCopyBuffer(command_buffer, src_buffer, m_buffer, 1, &copy_region);

		r_device.endSingleTimeCommands(command_buffer);
	}

	void Buffer::map()
	{
		vkMapMemory(r_device.device(), m_memory, 0, m_size, 0, &p_data);
	}

	void Buffer::writeToBuffer(void* data)
	{
		memcpy(p_data, data, m_size);
	}


	void Buffer::createBuffer(const VkDeviceSize& size,
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
		alloc_info.memoryTypeIndex = r_device.findMemoryType(memory_reqs.memoryTypeBits, props);

		if (vkAllocateMemory(r_device.device(), &alloc_info, nullptr, &buffer_memory ) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate buffer memory");

		vkBindBufferMemory(r_device.device(), buffer, buffer_memory, 0);
	}
	
}