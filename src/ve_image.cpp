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
    VkMemoryPropertyFlags properties,
    VmaMemoryUsage vmaUsage /*= VMA_MEMORY_USAGE_CPU_TO_GPU*/) {

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = vmaUsage;
  allocInfo.requiredFlags = properties;

  auto result = vmaCreateImage(m_allocator, createInfo, &allocInfo, &image, &allocation, nullptr);
  if (result != VK_SUCCESS) {
    std::cout << "result: " << result << std::endl;
    std::cout << "flags: " << createInfo->flags << std::endl;
    std::cout << "imageType: " << createInfo->imageType << std::endl;
    std::cout << "format: " << createInfo->format << std::endl;
    std::cout << "extent: " << createInfo->extent.width << " " << createInfo->extent.height << " "
              << createInfo->extent.depth << std::endl;
    std::cout << "format: " << createInfo->format << std::endl;
    std::cout << "mipLevels: " << createInfo->mipLevels << std::endl;
    std::cout << "arrayLayers: " << createInfo->arrayLayers << std::endl;
    std::cout << "samples: " << createInfo->samples << std::endl;
    std::cout << "tiling: " << createInfo->tiling << std::endl;
    std::cout << "usage: " << createInfo->usage << std::endl;
    std::cout << "sharingMode: " << createInfo->sharingMode << std::endl;
    std::cout << "initialLayout: " << createInfo->initialLayout << std::endl;

    throw std::runtime_error("Failed to create image!");
  }
}

} // namespace ve