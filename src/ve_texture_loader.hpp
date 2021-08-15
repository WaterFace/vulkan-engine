#pragma once

#include "ve_device.hpp"
#include "ve_image.hpp"
#include "ve_texture.hpp"

#include <memory>
#include <vector>

namespace ve {

struct LoadedTexture {
  std::unique_ptr<Image> image;
  VkImageView imageView;
  uint32_t width, height;
  uint32_t mipLevels;
  uint32_t layerCount;
  VkSampler sampler;
};

class TextureLoader {
public:
  TextureLoader(Device &device);
  ~TextureLoader();

  static const std::string TEXTURE_PATH;

  const std::vector<VkDescriptorImageInfo> &descriptorInfos() { return m_descriptorInfos; }
  const uint32_t descriptorCount() { return static_cast<uint32_t>(m_descriptorInfos.size()); }
  const VkDescriptorImageInfo &globalSamplerInfo() { return m_globalSamplerInfo; }

  Texture loadFromFile(const std::string &path);

private:
  Device &m_device;

  VkSampler m_globalSampler;
  VkDescriptorImageInfo m_globalSamplerInfo{};

  std::vector<VkDescriptorImageInfo> m_descriptorInfos;

  // TODO: cache textures
  std::vector<LoadedTexture> m_loadedTextures;
};

} // namespace ve