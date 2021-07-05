#pragma once

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace ve {

class Buffer {
public:
  VkBuffer buffer;
  VmaAllocation allocation;

  VkResult bind(VmaAllocator allocator);
  void write(VmaAllocator allocator, void *data, VkDeviceSize size);
};

} // namespace ve