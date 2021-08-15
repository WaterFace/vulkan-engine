#include "ve_texture_loader.hpp"

#include "tinygltf/stb_image.h"

#include <iostream>
#include <stdexcept>

namespace ve {

const std::string TextureLoader::TEXTURE_PATH = "textures/";

TextureLoader::TextureLoader(Device &device)
    : m_device{device} {
  VkSamplerCreateInfo globalSamplerCreateInfo = Texture::defaultSamplerInfo();
  vkCreateSampler(m_device.device(), &globalSamplerCreateInfo, nullptr, &m_globalSampler);
  m_globalSamplerInfo.sampler = m_globalSampler;
}
TextureLoader::~TextureLoader() {
  for (auto &t : m_loadedTextures) {
    vkDestroyImageView(m_device.device(), t.imageView, nullptr);
    vkDestroySampler(m_device.device(), t.sampler, nullptr);
  }
  vkDestroySampler(m_device.device(), m_globalSampler, nullptr);
}

Texture TextureLoader::loadFromFile(const std::string &path) {
  std::cout << "Loading texture " << path << std::endl;

  int width, height, channels;

  std::string fullPath = TEXTURE_PATH + path;

  stbi_uc *pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("Failed to load texture " + path + "!");
    return {SIZE_MAX};
  }

  void *pixelPtr = pixels;
  VkDeviceSize stagingBufferSize = width * height * 4;
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

  Buffer stagingBuffer{m_device.getAllocator()};
  stagingBuffer.create(stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingBuffer.write(pixelPtr, stagingBufferSize);

  stbi_image_free(pixels);

  VkExtent3D imageExtent{};
  imageExtent.width = static_cast<uint32_t>(width);
  imageExtent.height = static_cast<uint32_t>(height);
  imageExtent.depth = 1;

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.pNext = nullptr;

  imageInfo.extent = imageExtent;
  imageInfo.format = format;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  auto newImage = std::make_unique<Image>(m_device.getAllocator());
  newImage->create(&imageInfo, 0, VMA_MEMORY_USAGE_GPU_ONLY);

  // Layout transition:
  VkCommandBuffer cmd = m_device.beginSingleTimeCommands();

  VkImageSubresourceRange range{};
  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.layerCount = imageInfo.arrayLayers;
  range.baseArrayLayer = 0;
  range.levelCount = imageInfo.mipLevels;
  range.baseMipLevel = 0;

  VkImageMemoryBarrier imageBarrierToTransfer{};
  imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageBarrierToTransfer.pNext = nullptr;

  imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageBarrierToTransfer.image = newImage->image;
  imageBarrierToTransfer.subresourceRange = range;

  imageBarrierToTransfer.srcAccessMask = 0;
  imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  vkCmdPipelineBarrier(
      cmd,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageBarrierToTransfer);

  m_device.endSingleTimeCommands(cmd);

  m_device.copyBufferToImage(stagingBuffer.buffer, newImage->image, width, height, 1);

  cmd = m_device.beginSingleTimeCommands();

  VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
  imageBarrierToReadable.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageBarrierToReadable.pNext = nullptr;

  imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageBarrierToReadable.image = newImage->image;
  imageBarrierToReadable.subresourceRange = range;

  imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(
      cmd,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &imageBarrierToReadable);

  m_device.endSingleTimeCommands(cmd);

  VkImageViewCreateInfo imageViewInfo{};
  imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewInfo.pNext = nullptr;

  imageViewInfo.image = newImage->image;
  imageViewInfo.flags = 0;
  imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.format = format;
  imageViewInfo.subresourceRange = range;

  VkImageView textureImageView;
  vkCreateImageView(m_device.device(), &imageViewInfo, nullptr, &textureImageView);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.pNext = nullptr;

  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  VkSampler textureSampler;
  vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &textureSampler);

  LoadedTexture newTexture{
      std::move(newImage),
      textureImageView,
      width,
      height,
      imageInfo.mipLevels,
      imageInfo.arrayLayers,
      textureSampler,
  };

  size_t id = m_loadedTextures.size();
  m_loadedTextures.push_back(std::move(newTexture));

  VkDescriptorImageInfo descriptorInfo{};
  descriptorInfo.sampler = textureSampler;
  descriptorInfo.imageView = textureImageView;
  descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  m_descriptorInfos.push_back(descriptorInfo);

  return {id};
}

} // namespace ve