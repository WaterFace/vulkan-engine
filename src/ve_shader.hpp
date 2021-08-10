#pragma once

#include "ve_device.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ve {

class ShaderStage {
public:
  ShaderStage(Device &device, const std::string &filepath, VkShaderStageFlagBits stage);
  ~ShaderStage();

  const VkShaderModule module() { return m_shaderModule; }
  const VkShaderStageFlagBits stage() { return m_stage; }
  const std::vector<uint8_t> &code() { return m_code; }

private:
  static std::vector<uint8_t> readFile(const std::string &filepath);
  void createShaderModule(const std::vector<uint8_t> &code, VkShaderModule *shaderModule);

  Device &m_device;
  VkShaderModule m_shaderModule;
  VkShaderStageFlagBits m_stage;
  std::vector<uint8_t> m_code;
};

class ShaderEffect {
public:
  ShaderEffect(Device &device);
  ~ShaderEffect();

  void addStage(std::shared_ptr<ShaderStage> shader);
  std::vector<VkPipelineShaderStageCreateInfo> fillStages();

  VkPipelineLayout reflectLayout();

private:
  Device &m_device;
  std::vector<std::shared_ptr<ShaderStage>> m_stages;
  struct ReflectedBinding {
    uint32_t set;
    uint32_t binding;
    VkDescriptorType type;
  };
  std::unordered_map<std::string, ReflectedBinding> m_bindings;

  std::array<VkDescriptorSetLayout, 4> m_setLayouts;
  std::array<uint32_t, 4> m_setHashes;

  // VkPipelineLayout m_pipelineLayout;
};

} // namespace ve