#pragma once

#include "ve_device.hpp"

#include <string>
#include <vector>

namespace ve {

class Pipeline {
public:
  Pipeline(Device &device, VkPipeline pipeline);
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;

  void bind(VkCommandBuffer cmd);

private:
  VkPipeline m_graphicsPipeline;
  Device &m_device;
};

} // namespace ve