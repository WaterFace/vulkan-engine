#include "ve_shader.hpp"

#include <fstream>
#include <iostream>

namespace ve {

Shader::Shader(Device &device, const std::string &filepath) : m_device{device} {
  auto code = readFile(filepath);
  std::cout << filepath << " file size: " << code.size() << std::endl;
  createShaderModule(code, &m_shaderModule);
}

Shader::~Shader() { vkDestroyShaderModule(m_device.device(), m_shaderModule, nullptr); }

std::vector<char> Shader::readFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void Shader::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = code.size();
  info.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(m_device.device(), &info, nullptr, shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
}

} // namespace ve