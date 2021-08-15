#pragma once

#include <vulkan/vulkan.h>

#include "vk_mem_alloc/vk_mem_alloc.h"

namespace ve {

class Image {
public:
  Image(VmaAllocator allocator) : image{VK_NULL_HANDLE}, allocation{VK_NULL_HANDLE}, m_allocator{allocator} {};
  ~Image();

  VkImage image;
  VmaAllocation allocation;

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  void create(
      VkImageCreateInfo *createInfo, VkMemoryPropertyFlags properties,
      VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

private:
  VmaAllocator m_allocator;
  bool m_memoryMapped{false};
  uint8_t *m_data;
};

} // namespace ve