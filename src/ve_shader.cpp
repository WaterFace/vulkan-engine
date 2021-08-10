#include "ve_shader.hpp"

#include "spirv_reflect/spirv_reflect.h"

#include <cassert>
#include <fstream>
#include <iostream>

namespace ve {

ShaderStage::ShaderStage(Device &device, const std::string &filepath, VkShaderStageFlagBits stage)
    : m_device{device}
    , m_stage{stage} {
  auto code = readFile(filepath);
  std::cout << filepath << " file size: " << code.size() << std::endl;
  createShaderModule(code, &m_shaderModule);
  m_code = std::move(code);
}

ShaderStage::~ShaderStage() { vkDestroyShaderModule(m_device.device(), m_shaderModule, nullptr); }

std::vector<uint8_t> ShaderStage::readFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<uint8_t> buffer(fileSize);

  file.seekg(0);
  file.read((char *)buffer.data(), fileSize);
  file.close();

  return buffer;
}

void ShaderStage::createShaderModule(const std::vector<uint8_t> &code, VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = code.size();
  info.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(m_device.device(), &info, nullptr, shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
}

ShaderEffect::ShaderEffect(Device &device)
    : m_device{device} {};

ShaderEffect::~ShaderEffect() {
  for (auto setLayout : m_setLayouts) {
    vkDestroyDescriptorSetLayout(m_device.device(), setLayout, nullptr);
  }
}

void ShaderEffect::addStage(std::shared_ptr<ShaderStage> shader) { m_stages.push_back(shader); }

struct DescriptorSetLayoutData {
  uint32_t setNumber;
  VkDescriptorSetLayoutCreateInfo info;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};
VkPipelineLayout ShaderEffect::reflectLayout() {
  std::vector<DescriptorSetLayoutData> setLayouts;
  std::vector<VkPushConstantRange> pushConstantRanges;

  for (auto &s : m_stages) {
    SpvReflectShaderModule spvModule;
    SpvReflectResult result =
        spvReflectCreateShaderModule(s->code().size() * sizeof(uint8_t), s->code().data(), &spvModule);

    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&spvModule, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectDescriptorSet *> sets(count);
    result = spvReflectEnumerateDescriptorSets(&spvModule, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
      const SpvReflectDescriptorSet &reflectedSet = *(sets[i_set]);

      DescriptorSetLayoutData layoutData{};

      layoutData.bindings.resize(reflectedSet.binding_count);
      for (uint32_t i_binding = 0; i_binding < reflectedSet.binding_count; i_binding++) {
        const SpvReflectDescriptorBinding &reflectedBinding = *(reflectedSet.bindings[i_binding]);

        VkDescriptorSetLayoutBinding &layoutBinding = layoutData.bindings[i_binding];
        layoutBinding.binding = reflectedBinding.binding;
        layoutBinding.descriptorType = static_cast<VkDescriptorType>(reflectedBinding.descriptor_type);

        layoutBinding.descriptorCount = 1;
        for (uint32_t i_dim = 0; i_dim < reflectedBinding.array.dims_count; ++i_dim) {
          layoutBinding.descriptorCount *= reflectedBinding.array.dims[i_dim];
        }

        layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(spvModule.shader_stage);

        ReflectedBinding reflected;
        reflected.binding = layoutBinding.binding;
        reflected.set = reflectedSet.set;
        reflected.type = layoutBinding.descriptorType;

        m_bindings[reflectedBinding.name] = reflected;
      }

      layoutData.setNumber = reflectedSet.set;
      layoutData.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutData.info.bindingCount = reflectedSet.binding_count;
      layoutData.info.pBindings = layoutData.bindings.data();

      setLayouts.push_back(layoutData);
    }

    // Push constants
    result = spvReflectEnumeratePushConstantBlocks(&spvModule, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectBlockVariable *> pushConstants(count);
    result = spvReflectEnumeratePushConstantBlocks(&spvModule, &count, pushConstants.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    if (count > 0) {
      VkPushConstantRange range{};
      range.offset = pushConstants[0]->offset;
      range.size = pushConstants[0]->size;
      range.stageFlags = s->stage();

      pushConstantRanges.push_back(range);
    }
  }

  std::array<DescriptorSetLayoutData, 4> mergedLayouts;
  for (int i = 0; i < 4; i++) {
    DescriptorSetLayoutData &layout = mergedLayouts[i];

    layout.setNumber = i;
    layout.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    std::unordered_map<int, VkDescriptorSetLayoutBinding> bindings;

    for (auto &s : setLayouts) {
      if (s.setNumber != i) {
        continue;
      }

      for (auto &b : s.bindings) {
        auto it = bindings.find(b.binding);
        if (it == bindings.end()) {
          bindings[b.binding] = b;
        } else {
          // merge flags
          bindings[b.binding].stageFlags |= b.stageFlags;
        }
      }
    }
    for (auto [k, v] : bindings) {
      layout.bindings.push_back(v);
    }

    layout.info.bindingCount = static_cast<uint32_t>(layout.bindings.size());
    layout.info.pBindings = layout.bindings.data();
    layout.info.flags = 0;
    layout.info.pNext = nullptr;

    if (layout.info.bindingCount > 0) {
      vkCreateDescriptorSetLayout(m_device.device(), &layout.info, nullptr, &m_setLayouts[i]);
    } else {
      m_setLayouts[i] = VK_NULL_HANDLE;
    }
  }

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  layoutInfo.pPushConstantRanges = pushConstantRanges.data();
  layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());

  std::array<VkDescriptorSetLayout, 4> compactedLayouts;
  int s = 0;
  for (int i = 0; i < 4; i++) {
    if (m_setLayouts[i] != VK_NULL_HANDLE) {
      compactedLayouts[s] = m_setLayouts[i];
      s++;
    }
  }

  layoutInfo.setLayoutCount = s;
  layoutInfo.pSetLayouts = compactedLayouts.data();

  VkPipelineLayout layout{};

  if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to reflect pipeline layout!");
  }

  return layout;
}

} // namespace ve
