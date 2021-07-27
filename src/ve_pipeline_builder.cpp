#include "ve_pipeline_builder.hpp"

#include <iostream>
#include <stdexcept>

namespace ve {

PipelineBuilder::PipelineBuilder(Device &device)
    : m_device{device}
    , m_shaderEffect{device} {
  defaultPipelineConfigInfo(m_configInfo);
}

PipelineBuilder &PipelineBuilder::addShaderStage(std::shared_ptr<ShaderStage> shader) {
  VkPipelineShaderStageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.module = shader->module();
  info.stage = shader->stage();
  info.pName = "main"; // TODO: maybe don't hardcode this?
  info.flags = 0;
  info.pNext = nullptr;

  m_shaderStages.push_back(info);
  m_shaderEffect.addStage(shader);

  return *this;
}

PipelineBuilder &PipelineBuilder::setVertexInput(
    const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions) {
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

  m_configInfo.vertexInputInfo = vertexInputInfo;

  return *this;
}

PipelineBuilder &PipelineBuilder::setPrimitiveTopology(VkPrimitiveTopology topology) {
  m_configInfo.inputAssemblyInfo.topology = topology;

  return *this;
}

PipelineBuilder &PipelineBuilder::setRenderPass(VkRenderPass renderPass) {
  m_configInfo.renderPass = renderPass;

  return *this;
}

PipelineBuilder &PipelineBuilder::setSampleCount(VkSampleCountFlagBits sampleCount) {
  m_configInfo.multisampleInfo.rasterizationSamples = sampleCount;

  return *this;
}

PipelineBuilder &PipelineBuilder::setLayout(VkPipelineLayout layout) {
  m_pipelineLayout = layout;

  return *this;
}

PipelineBuilder &PipelineBuilder::reflectLayout() {
  m_pipelineLayout = m_shaderEffect.reflectLayout();

  return *this;
}

std::unique_ptr<Pipeline> PipelineBuilder::build() {
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
  pipelineInfo.pStages = m_shaderStages.data();
  pipelineInfo.pVertexInputState = &m_configInfo.vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &m_configInfo.inputAssemblyInfo;
  pipelineInfo.pViewportState = &m_configInfo.viewportInfo;
  pipelineInfo.pRasterizationState = &m_configInfo.rasterizationInfo;
  pipelineInfo.pMultisampleState = &m_configInfo.multisampleInfo;
  pipelineInfo.pColorBlendState = &m_configInfo.colorBlendInfo;
  pipelineInfo.pDepthStencilState = &m_configInfo.depthStencilInfo;
  pipelineInfo.pDynamicState = &m_configInfo.dynamicStateInfo;

  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = m_configInfo.renderPass;
  pipelineInfo.subpass = m_configInfo.subpass;

  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VkPipeline pipeline{};

  if (vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline");
  }

  return std::move(std::make_unique<Pipeline>(m_device, pipeline, m_pipelineLayout));
}

void PipelineBuilder::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo) {
  configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.pViewports = nullptr;
  configInfo.viewportInfo.scissorCount = 1;
  configInfo.viewportInfo.pScissors = nullptr;

  configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  configInfo.rasterizationInfo.lineWidth = 1.0f;
  configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

  configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  configInfo.multisampleInfo.minSampleShading = 1.0f;
  configInfo.multisampleInfo.pSampleMask = nullptr;
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

  configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f;
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {};
  configInfo.depthStencilInfo.back = {};

  configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
  configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
  configInfo.dynamicStateInfo.flags = 0;
}

} // namespace ve