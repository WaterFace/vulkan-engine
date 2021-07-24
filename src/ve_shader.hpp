#pragma once

#include "ve_device.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace ve {

class Shader {
public:
  Shader(Device &device, const std::string &filepath);
  ~Shader();

  const VkShaderModule module() { return m_shaderModule; }

private:
  static std::vector<char> readFile(const std::string &filepath);
  void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

  Device &m_device;
  VkShaderModule m_shaderModule;
};

} // namespace ve