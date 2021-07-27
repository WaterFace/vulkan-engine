#pragma once

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace ve {

class Buffer {
public:
  Buffer(VmaAllocator allocator)
      : buffer{VK_NULL_HANDLE}
      , allocation{VK_NULL_HANDLE}
      , m_allocator{allocator} {};
  ~Buffer();

  VkBuffer buffer;
  VmaAllocation allocation;

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  void write(void *data, VkDeviceSize size);
  void write(void *data, VkDeviceSize size, VkDeviceSize offset);

  void *data() { return m_data; }
  void mapMemory();
  void unmapMemory();
  void create(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

private:
  VmaAllocator m_allocator;
  bool m_memoryMapped{false};
  uint8_t *m_data;
};

} // namespace ve