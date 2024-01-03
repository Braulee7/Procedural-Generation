#pragma once
#include "evn_device.h"
namespace evn {
	class Buffer {
	public:
		Buffer(Device& device, 
			const VkDeviceSize& size,
			const VkBufferUsageFlags& usage,
			const VkMemoryPropertyFlags& properties);
		~Buffer();
		inline VkBuffer& getBuffer() { return m_buffer; }
		void copyBuffer(VkBuffer& src_buffer, const VkDeviceSize& size);
		void map(void* data);
	private:
		void createBuffer(const VkDeviceSize& size,
			const VkBufferUsageFlags& usage,
			const VkMemoryPropertyFlags& props,
			VkBuffer& buffer,
			VkDeviceMemory& buffer_memory);

	private:
		Device& r_device;
		VkBuffer m_buffer;
		VkDeviceMemory m_memory;
		VkDeviceSize m_size;
		void* p_data;
	};
}