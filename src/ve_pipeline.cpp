#include "ve_pipeline.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace ve {

Pipeline::Pipeline(Device &device, VkPipeline pipeline, VkPipelineLayout layout)
    : m_device{device}
    , m_graphicsPipeline{pipeline}
    , m_layout{layout} {}

Pipeline::~Pipeline() {
  vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(m_device.device(), m_layout, nullptr);
}

void Pipeline::bind(VkCommandBuffer cmd) {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
}

} // namespace ve