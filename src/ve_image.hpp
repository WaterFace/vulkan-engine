#pragma once

#include <vulkan/vulkan.h>

#include "vk_mem_alloc/vk_mem_alloc.h"

namespace ve {

class Image {
public:
  Image(VmaAllocator allocator)
      : image{VK_NULL_HANDLE}
      , allocation{VK_NULL_HANDLE}
      , m_allocator{allocator} {};
  ~Image();

  VkImage image;
  VmaAllocation allocation;

  VkImageViewCreateInfo imageViewInfo() { return m_imageViewInfo; }

  const VkImageType imageType() { return m_imageType; }
  const VkFormat format() { return m_format; }
  const VkExtent3D extent() { return m_extent; }
  const uint32_t mipLevels() { return m_mipLevels; }
  const uint32_t arrayLayers() { return m_arrayLayers; }
  const VkSampleCountFlags samples() { return m_samples; }

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  void create(
      VkImageCreateInfo *createInfo,
      VkImageAspectFlags imageAspect,
      VkMemoryPropertyFlags properties,
      VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

private:
  VkImageSubresourceRange m_subresourceRange{};
  VkImageViewCreateInfo m_imageViewInfo{};

  VkImageAspectFlags m_imageAspect{};

  VkImageType m_imageType{};
  VkFormat m_format{};
  VkExtent3D m_extent{};
  uint32_t m_mipLevels{};
  uint32_t m_arrayLayers{};
  VkSampleCountFlags m_samples{};

  VmaAllocator m_allocator;
  bool m_memoryMapped{false};
  uint8_t *m_data;
};

} // namespace ve