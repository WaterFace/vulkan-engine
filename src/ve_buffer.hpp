#pragma once

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace ve {

class Buffer {
public:
  Buffer(VmaAllocator allocator) : buffer{VK_NULL_HANDLE}, allocation{VK_NULL_HANDLE}, m_allocator{allocator} {};
  ~Buffer();

  VkBuffer buffer;
  VmaAllocation allocation;

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  void write(void *data, VkDeviceSize size);
  void create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

private:
  VmaAllocator m_allocator;
};

} // namespace ve