#include "ve_image.hpp"

#include "vk_mem_alloc/vk_mem_alloc.h"

#include <cassert>
#include <cstring>

namespace ve {

Image::~Image() { vmaDestroyImage(m_allocator, image, allocation); }

void Image::write(void *contents, VkDeviceSize size, VkDeviceSize offset) {
  uint8_t *data;
  vmaMapMemory(m_allocator, allocation, (void **)&data);
  memcpy(&data[offset], contents, static_cast<size_t>(size));
  vmaUnmapMemory(m_allocator, allocation);
}

void Image::write(void *contents, VkDeviceSize size) { write(contents, size, 0); }

// void Image::writeMapped(void *contents, VkDeviceSize size, VkDeviceSize offset) {
//   assert(m_memoryMapped && "Tried to write to memory that hasn't been mapped");
//   memcpy(&m_data[offset], contents, static_cast<size_t>(size));
// }

void Image::mapMemory() {
  vmaMapMemory(m_allocator, allocation, (void **)&m_data);
  m_memoryMapped = true;
}

void Image::unmapMemory() {
  vmaUnmapMemory(m_allocator, allocation);
  m_memoryMapped = false;
}

void Image::create(
    VkDeviceSize size,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VmaMemoryUsage vmaUsage /*= VMA_MEMORY_USAGE_CPU_TO_GPU*/) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  imageInfo.pNext = nullptr;
  imageInfo.flags = 0;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  imageInfo.extent = {0, 0, 0};
  imageInfo.mipLevels = 0;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.queueFamilyIndexCount = 0;
  imageInfo.pQueueFamilyIndices = nullptr;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = vmaUsage;
  allocInfo.requiredFlags = properties;

  vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr);
}

} // namespace ve