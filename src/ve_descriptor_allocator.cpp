#include "ve_descriptor_allocator.hpp"

namespace ve {

DescriptorAllocator::DescriptorAllocator(VkDevice newDevice) : device{newDevice} {}
DescriptorAllocator::~DescriptorAllocator() {
  for (auto p : m_usedPools) {
    vkDestroyDescriptorPool(device, p, nullptr);
  }
  for (auto p : m_freePools) {
    vkDestroyDescriptorPool(device, p, nullptr);
  }
}

VkDescriptorPool createPool(VkDevice device, DescriptorAllocator::PoolSizes &poolSizes, int count,
                            VkDescriptorPoolCreateFlags flags) {
  std::vector<VkDescriptorPoolSize> sizes;
  sizes.reserve(poolSizes.sizes.size());
  for (auto sz : poolSizes.sizes) {
    sizes.push_back({sz.first, uint32_t(sz.second * count)});
  }

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = flags;
  poolInfo.maxSets = count;
  poolInfo.poolSizeCount = (uint32_t)sizes.size();
  poolInfo.pPoolSizes = sizes.data();

  VkDescriptorPool descriptorPool;
  vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

  return descriptorPool;
}

VkDescriptorPool DescriptorAllocator::grabPool() {
  if (m_freePools.size() > 0) {
    VkDescriptorPool pool = m_freePools.back();
    m_freePools.pop_back();
    return pool;
  } else {
    return createPool(device, m_descriptorSizes, 1000, 0);
  }
}

bool DescriptorAllocator::allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout) {
  if (m_currentPool == VK_NULL_HANDLE) {
    m_currentPool = grabPool();
    m_usedPools.push_back(m_currentPool);
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.pSetLayouts = &layout;
  allocInfo.descriptorPool = m_currentPool;
  allocInfo.descriptorSetCount = 1;

  VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
  bool needReallocate = false;

  switch (allocResult) {
  case VK_SUCCESS:
    return true;
  case VK_ERROR_FRAGMENTED_POOL:
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    needReallocate = true;
    break;
  default:
    return false;
  }

  if (needReallocate) {
    m_currentPool = grabPool();
    m_usedPools.push_back(m_currentPool);

    allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);

    if (allocResult == VK_SUCCESS) {
      return true;
    }
  }
  return false;
}

void DescriptorAllocator::resetPools() {
  for (auto p : m_usedPools) {
    vkResetDescriptorPool(device, p, 0);
  }
  m_freePools = m_usedPools;
  m_usedPools.clear();

  m_currentPool = VK_NULL_HANDLE;
}

} // namespace ve