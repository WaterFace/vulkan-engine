#include "ve_pipeline.hpp"

#include "ve_model.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace ve {

Pipeline::Pipeline(Device &device, VkPipeline pipeline) : m_device{device}, m_graphicsPipeline{pipeline} {}

Pipeline::~Pipeline() { vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr); }

void Pipeline::bind(VkCommandBuffer cmd) {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
}

} // namespace ve