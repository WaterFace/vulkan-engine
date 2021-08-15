#pragma once

#include "ve_descriptor_allocator.hpp"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

namespace ve {

class DescriptorLayoutCache {
public:
  DescriptorLayoutCache(VkDevice device);
  ~DescriptorLayoutCache();

  VkDescriptorSetLayout createDescriptorLayout(VkDescriptorSetLayoutCreateInfo *info);

  struct DescriptorLayoutInfo {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    bool operator==(const DescriptorLayoutInfo &other) const;

    uint64_t hash() const;
  };

private:
  struct DescriptorLayoutHash {
    uint64_t operator()(const DescriptorLayoutInfo &k) const { return k.hash(); }
  };

  std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_cache;
  VkDevice m_device;
};

class DescriptorBuilder {
public:
  static DescriptorBuilder begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator);

  DescriptorBuilder &bindBuffer(
      uint32_t binding,
      VkDescriptorBufferInfo *bufferInfo,
      VkDescriptorType type,
      VkShaderStageFlags stageFlags);
  DescriptorBuilder &bindImage(
      uint32_t binding,
      const VkDescriptorImageInfo *imageInfo,
      VkDescriptorType type,
      VkShaderStageFlags stageFlags);
  DescriptorBuilder &bindImages(
      uint32_t binding,
      uint32_t count,
      const VkDescriptorImageInfo *imageInfo,
      VkDescriptorType type,
      VkShaderStageFlags stageFlags);
  DescriptorBuilder &bindSamplers(
      uint32_t binding,
      uint32_t count,
      const VkDescriptorImageInfo *imageInfo,
      VkShaderStageFlags stageFlags);
  DescriptorBuilder &bindCombinedSampler(
      uint32_t binding,
      uint32_t count,
      const VkDescriptorImageInfo *imageInfo,
      VkShaderStageFlags stageFlags);

  bool build(VkDescriptorSet &set, VkDescriptorSetLayout &layout);
  bool build(VkDescriptorSet &set);

private:
  std::vector<VkWriteDescriptorSet> m_writes;
  std::vector<VkDescriptorSetLayoutBinding> m_bindings;

  DescriptorLayoutCache *m_cache;
  DescriptorAllocator *m_allocator;
};

} // namespace ve