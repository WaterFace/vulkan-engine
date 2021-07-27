#pragma once

#include "ve_device.hpp"
#include "ve_pipeline.hpp"
#include "ve_shader.hpp"

#include <memory>
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
  VkRenderPass renderPass = VK_NULL_HANDLE;
  uint32_t subpass = 0;
};

class PipelineBuilder {
public:
  PipelineBuilder(Device &device);

  PipelineBuilder &addShaderStage(std::shared_ptr<ShaderStage> shader);
  PipelineBuilder &setVertexInput(
      const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
      const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
  PipelineBuilder &setPrimitiveTopology(VkPrimitiveTopology topology);
  PipelineBuilder &setRenderPass(VkRenderPass renderPass);
  PipelineBuilder &setSampleCount(VkSampleCountFlagBits sampleCount);
  PipelineBuilder &setLayout(VkPipelineLayout layout);
  PipelineBuilder &reflectLayout();

  std::unique_ptr<Pipeline> build();

private:
  static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
  PipelineConfigInfo m_configInfo{};
  VkPipelineLayout m_pipelineLayout;
  Device &m_device;
  ShaderEffect m_shaderEffect;
};

} // namespace ve