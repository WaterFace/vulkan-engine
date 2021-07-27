#pragma once

#include "ve_device.hpp"

#include <memory>
#include <string>
#include <vector>

namespace ve {

class Pipeline {
public:
  Pipeline(Device &device, VkPipeline pipeline, VkPipelineLayout layout);
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;

  VkPipelineLayout &layout() { return m_layout; }

  void bind(VkCommandBuffer cmd);

private:
  VkPipeline m_graphicsPipeline;
  VkPipelineLayout m_layout;
  Device &m_device;
};

} // namespace ve