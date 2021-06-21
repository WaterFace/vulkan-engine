#pragma once

#include "ve_device.hpp"

#include <string>
#include <vector>

namespace ve {

struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  uint32_t subpass = 0;
};

class Pipeline {
  public:
  Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath,
      const PipelineConfigInfo& configInfo);
  ~Pipeline();

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

  void bind(VkCommandBuffer cmd);

  private:
  static std::vector<char> readFile(const std::string& filepath);

  void createGraphicsPipeline(
      const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);

  void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

  Device& m_device;
  VkPipeline m_graphicsPipeline;
  VkShaderModule m_vertShaderModule;
  VkShaderModule m_fragShaderModule;
};

} // namespace ve