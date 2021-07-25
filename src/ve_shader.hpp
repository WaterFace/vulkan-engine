#pragma once

#include "ve_device.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace ve {

class Shader {
public:
  Shader(Device &device, const std::string &filepath);
  ~Shader();

  const VkShaderModule module() { return m_shaderModule; }

private:
  static std::vector<uint8_t> readFile(const std::string &filepath);
  void createShaderModule(const std::vector<uint8_t> &code, VkShaderModule *shaderModule);

  Device &m_device;
  VkShaderModule m_shaderModule;
  std::vector<uint8_t> m_code;
};

struct ShaderModule {
  std::vector<uint8_t> code;
  VkShaderModule module;
};

class ShaderEffect {
public:
  void addStage(ShaderModule *shaderModule, VkShaderStageFlagBits stage);
  std::vector<VkPipelineShaderStageCreateInfo> fillStages();

  void reflectLayout();

private:
  Device &m_device;
  struct ShaderStage {
    ShaderModule *module;
    VkShaderStageFlagBits stage;
  };
  std::vector<ShaderStage> m_stages;
  struct ReflectedBinding {
    uint32_t set;
    uint32_t binding;
    VkDescriptorType type;
  };
  std::unordered_map<std::string, ReflectedBinding> m_bindings;

  std::array<VkDescriptorSetLayout, 4> m_setLayouts;
  std::array<uint32_t, 4> m_setHashes;

  VkPipelineLayout m_pipelineLayout;
};

} // namespace ve