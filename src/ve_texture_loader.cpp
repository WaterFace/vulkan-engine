#include "ve_texture_loader.hpp"

#include "stb_image/stb_image.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace ve {

const std::string TextureLoader::TEXTURE_PATH = "textures/";
const uint32_t TextureLoader::MAX_TEXTURES = 1000;

TextureLoader::TextureLoader(Device &device)
    : m_device{device} {
  VkSamplerCreateInfo globalSamplerCreateInfo = Texture::defaultSamplerInfo();
  vkCreateSampler(m_device.device(), &globalSamplerCreateInfo, nullptr, &m_globalSampler);
  m_globalSamplerInfo.sampler = m_globalSampler;

  uint8_t blackPixel[4] = {0, 0, 0, 255};
  m_textureCache[""] = loadFromData(blackPixel, 1, 1);

  m_descriptorInfos.resize(MAX_TEXTURES, m_descriptorInfos[0]);
}
TextureLoader::~TextureLoader() {
  for (auto &t : m_loadedTextures) {
    vkDestroyImageView(m_device.device(), t.imageView, nullptr);
    vkDestroySampler(m_device.device(), t.sampler, nullptr);
  }
  vkDestroySampler(m_device.device(), m_globalSampler, nullptr);
}

Texture TextureLoader::loadFromData(void *data, uint32_t width, uint32_t height) {
  assert(m_loadedTextures.size() < MAX_TEXTURES && "Maximum number of textures have been loaded");
  VkDeviceSize stagingBufferSize = width * height * 4;
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

  Buffer stagingBuffer{m_device.getAllocator()};
  stagingBuffer.create(stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingBuffer.write(data, stagingBufferSize);

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
  newImage->create(&imageInfo, VK_IMAGE_ASPECT_COLOR_BIT, 0, VMA_MEMORY_USAGE_GPU_ONLY);

  m_device.imageLayoutTransition(
      newImage->image,
      newImage->arrayLayers(),
      newImage->mipLevels(),
      VK_IMAGE_ASPECT_COLOR_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      0,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT);

  m_device.copyBufferToImage(stagingBuffer.buffer, newImage->image, width, height, 1);

  m_device.imageLayoutTransition(
      newImage->image,
      newImage->arrayLayers(),
      newImage->mipLevels(),
      VK_IMAGE_ASPECT_COLOR_BIT,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  VkImageViewCreateInfo imageViewInfo = newImage->imageViewInfo();

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

  if (id < m_descriptorInfos.size()) {
    m_descriptorInfos[id] = descriptorInfo;
  } else {
    m_descriptorInfos.push_back(descriptorInfo);
  }

  return {id};
}

Texture TextureLoader::loadFromFile(const std::string &path) {
  if (m_textureCache.find(path) != m_textureCache.end()) {
    std::cout << "TextureLoader: Loading texture " << path << " from cache" << std::endl;
    return m_textureCache[path];
  }

  std::cout << "TextureLoader: Loading texture " << path << " from disk" << std::endl;

  int width, height, channels;

  std::string fullPath = TEXTURE_PATH + path;

  stbi_uc *pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("Failed to load texture " + path + "!");
  }

  void *pixelPtr = pixels;

  Texture texture = loadFromData(pixelPtr, width, height);

  stbi_image_free(pixels);

  return texture;
}

} // namespace ve