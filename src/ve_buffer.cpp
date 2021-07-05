#include "ve_buffer.hpp"

#include <cassert>
#include <cstring>

namespace ve {

VkResult Buffer::bind(VmaAllocator allocator) { return vmaBindBufferMemory(allocator, allocation, buffer); }

void Buffer::write(VmaAllocator allocator, void *contents, VkDeviceSize size) {
  void *data;
  vmaMapMemory(allocator, allocation, &data);
  memcpy(data, contents, static_cast<size_t>(size));
  vmaUnmapMemory(allocator, allocation);
}

} // namespace ve