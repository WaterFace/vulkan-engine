#include "ve_descriptor_builder.hpp"

#include <algorithm>

namespace ve {

DescriptorLayoutCache::DescriptorLayoutCache(VkDevice device)
    : m_device{device} {}
DescriptorLayoutCache::~DescriptorLayoutCache() {
  for (auto pair : m_cache) {
    vkDestroyDescriptorSetLayout(m_device, pair.second, nullptr);
  }
}

VkDescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(VkDescriptorSetLayoutCreateInfo *info) {
  DescriptorLayoutInfo layoutInfo;
  layoutInfo.bindings.reserve(info->bindingCount);
  bool isSorted = true;
  int lastBinding = -1;

  for (uint32_t i = 0; i < info->bindingCount; i++) {
    layoutInfo.bindings.push_back(info->pBindings[i]);

    if (info->pBindings[i].binding > lastBinding) {
      lastBinding = info->pBindings[i].binding;
    } else {
      isSorted = false;
    }
  }

  if (!isSorted) {
    std::sort(
        layoutInfo.bindings.begin(),
        layoutInfo.bindings.end(),
        [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b) { return a.binding < b.binding; });
  }

  auto it = m_cache.find(layoutInfo);
  if (it != m_cache.end()) {
    return (*it).second;
  } else {
    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(m_device, info, nullptr, &layout);

    m_cache[layoutInfo] = layout;
    return layout;
  }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const {
  if (other.bindings.size() != bindings.size()) {
    return false;
  }

  // assumes the cache is sorted
  for (uint32_t i = 0; i < bindings.size(); i++) {
    if (other.bindings[i].binding != bindings[i].binding) {
      return false;
    }
    if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
      return false;
    }
    if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
      return false;
    }
    if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
      return false;
    }
  }
  return true;
}

uint64_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
  using std::hash;

  uint64_t result = hash<uint64_t>()(bindings.size());

  for (const VkDescriptorSetLayoutBinding &b : bindings) {
    // pack the binding data into a single int64. Not fully correct but it's ok
    uint64_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

    // shuffle the packed binding data and xor it with the main hash
    result ^= hash<uint64_t>()(binding_hash);
  }

  return result;
}

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator) {
  DescriptorBuilder builder;

  builder.m_cache = layoutCache;
  builder.m_allocator = allocator;
  return builder;
}

DescriptorBuilder &DescriptorBuilder::bindBuffer(
    uint32_t binding,
    VkDescriptorBufferInfo *bufferInfo,
    VkDescriptorType type,
    VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount = 1;
  newBinding.descriptorType = type;
  newBinding.stageFlags = stageFlags;
  newBinding.binding = binding;
  newBinding.pImmutableSamplers = nullptr;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext = nullptr;
  newWrite.descriptorCount = 1;
  newWrite.descriptorType = type;
  newWrite.pBufferInfo = bufferInfo;
  newWrite.dstBinding = binding;

  m_writes.push_back(newWrite);
  return *this;
}

DescriptorBuilder &DescriptorBuilder::bindImage(
    uint32_t binding,
    const VkDescriptorImageInfo *imageInfo,
    VkDescriptorType type,
    VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount = 1;
  newBinding.descriptorType = type;
  newBinding.stageFlags = stageFlags;
  newBinding.binding = binding;
  newBinding.pImmutableSamplers = nullptr;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext = nullptr;
  newWrite.descriptorCount = 1;
  newWrite.descriptorType = type;
  newWrite.pImageInfo = imageInfo;
  newWrite.dstBinding = binding;

  m_writes.push_back(newWrite);
  return *this;
}

DescriptorBuilder &DescriptorBuilder::bindImages(
    uint32_t binding,
    uint32_t count,
    const VkDescriptorImageInfo *imageInfo,
    VkDescriptorType type,
    VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount = count;
  newBinding.descriptorType = type;
  newBinding.stageFlags = stageFlags;
  newBinding.binding = binding;
  newBinding.pImmutableSamplers = nullptr;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext = nullptr;
  newWrite.descriptorCount = count;
  newWrite.descriptorType = type;
  newWrite.pImageInfo = imageInfo;
  newWrite.dstBinding = binding;

  m_writes.push_back(newWrite);
  return *this;
}

DescriptorBuilder &DescriptorBuilder::bindCombinedSamplers(
    uint32_t binding,
    uint32_t count,
    const VkDescriptorImageInfo *imageInfo,
    VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount = count;
  newBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  newBinding.stageFlags = stageFlags;
  newBinding.binding = binding;
  newBinding.pImmutableSamplers = nullptr;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext = nullptr;
  newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  newWrite.descriptorCount = count;
  newWrite.pImageInfo = imageInfo;
  newWrite.dstBinding = binding;

  m_writes.push_back(newWrite);
  return *this;
}

DescriptorBuilder &DescriptorBuilder::bindSamplers(
    uint32_t binding,
    uint32_t count,
    const VkDescriptorImageInfo *imageInfo,
    VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount = count;
  newBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  newBinding.stageFlags = stageFlags;
  newBinding.binding = binding;
  newBinding.pImmutableSamplers = nullptr;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext = nullptr;
  newWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  newWrite.descriptorCount = count;
  newWrite.pImageInfo = imageInfo;
  newWrite.dstBinding = binding;

  m_writes.push_back(newWrite);
  return *this;
}

bool DescriptorBuilder::build(VkDescriptorSet &set, VkDescriptorSetLayout &layout) {
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.pNext = nullptr;

  layoutInfo.pBindings = m_bindings.data();
  layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());

  layout = m_cache->createDescriptorLayout(&layoutInfo);

  bool success = m_allocator->allocate(&set, layout);
  if (!success) {
    return false;
  }

  for (VkWriteDescriptorSet &w : m_writes) {
    w.dstSet = set;
  }

  vkUpdateDescriptorSets(m_allocator->device, m_writes.size(), m_writes.data(), 0, nullptr);

  return true;
}

bool DescriptorBuilder::build(VkDescriptorSet &set) {
  VkDescriptorSetLayout layout{};
  return build(set, layout);
}

} // namespace ve