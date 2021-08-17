#include "ve_image.hpp"

#include "vk_mem_alloc/vk_mem_alloc.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace ve {

Image::~Image() { vmaDestroyImage(m_allocator, image, allocation); }

void Image::create(
    VkImageCreateInfo *createInfo,
    VkImageAspectFlags imageAspect,
    VkMemoryPropertyFlags properties,
    VmaMemoryUsage vmaUsage /*= VMA_MEMORY_USAGE_CPU_TO_GPU*/) {
  m_imageAspect = imageAspect;
  m_imageType = createInfo->imageType;
  m_format = createInfo->format;
  m_extent = createInfo->extent;
  m_mipLevels = createInfo->mipLevels;
  m_arrayLayers = createInfo->arrayLayers;
  m_samples = createInfo->samples;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = vmaUsage;
  allocInfo.requiredFlags = properties;

  auto result = vmaCreateImage(m_allocator, createInfo, &allocInfo, &image, &allocation, nullptr);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create image!");
  }

  VkImageSubresourceRange range{};
  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.layerCount = m_arrayLayers;
  range.baseArrayLayer = 0;
  range.levelCount = m_mipLevels;
  range.baseMipLevel = 0;

  m_subresourceRange = range;

  VkImageViewCreateInfo imageViewInfo{};
  imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewInfo.pNext = nullptr;

  imageViewInfo.image = image;
  imageViewInfo.flags = 0;
  imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.format = m_format;
  imageViewInfo.subresourceRange = m_subresourceRange;

  m_imageViewInfo = imageViewInfo;
}

} // namespace ve