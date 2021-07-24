#include "ve_buffer.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <cassert>
#include <cstring>

namespace ve {

Buffer::~Buffer() { vmaDestroyBuffer(m_allocator, buffer, allocation); }

void Buffer::write(void *contents, VkDeviceSize size) {
  void *data;
  vmaMapMemory(m_allocator, allocation, &data);
  memcpy(data, contents, static_cast<size_t>(size));
  vmaUnmapMemory(m_allocator, allocation);
}

void Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  // TODO: don't hardcode this
  allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  allocInfo.requiredFlags = properties;

  vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

} // namespace ve