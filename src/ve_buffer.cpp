#include "ve_buffer.hpp"

#include <cassert>
#include <cstring>

namespace ve {

Buffer::~Buffer() { vmaDestroyBuffer(m_allocator, buffer, allocation); }

void Buffer::write(void *contents, VkDeviceSize size, VkDeviceSize offset) {
  uint8_t *data;
  vmaMapMemory(m_allocator, allocation, (void **)&data);
  memcpy(&data[offset], contents, static_cast<size_t>(size));
  vmaUnmapMemory(m_allocator, allocation);
}

void Buffer::write(void *contents, VkDeviceSize size) { write(contents, size, 0); }

// void Buffer::writeMapped(void *contents, VkDeviceSize size, VkDeviceSize offset) {
//   assert(m_memoryMapped && "Tried to write to memory that hasn't been mapped");
//   memcpy(&m_data[offset], contents, static_cast<size_t>(size));
// }

void Buffer::mapMemory() {
  vmaMapMemory(m_allocator, allocation, (void **)&m_data);
  m_memoryMapped = true;
}

void Buffer::unmapMemory() {
  vmaUnmapMemory(m_allocator, allocation);
  m_memoryMapped = false;
}

void Buffer::create(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VmaMemoryUsage vmaUsage /*= VMA_MEMORY_USAGE_CPU_TO_GPU*/) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = vmaUsage;
  allocInfo.requiredFlags = properties;

  vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

} // namespace ve