#pragma once

#include "ve_device.hpp"

#include <vector>

#include <vulkan/vulkan.h>

namespace ve {

struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
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

class PipelineBuilder {
public:
  PipelineBuilder(Device &device);

  PipelineBuilder &addShaderStage(VkShaderModule module, VkShaderStageFlagBits stage);
  PipelineBuilder &setVertexInput(
      const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
      const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
  PipelineBuilder &setPrimitiveTopology(VkPrimitiveTopology topology);
  PipelineBuilder &setRenderPass(VkRenderPass renderPass);
  PipelineBuilder &setSampleCount(VkSampleCountFlagBits sampleCount);
  PipelineBuilder &setLayout(VkPipelineLayout layout);

  VkPipeline build();

private:
  static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
  PipelineConfigInfo m_configInfo{};
  Device &m_device;
};

} // namespace ve