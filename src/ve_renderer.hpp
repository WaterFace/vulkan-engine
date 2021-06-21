#pragma once

#include "ve_device.hpp"
#include "ve_swapchain.hpp"
#include "ve_window.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace ve {

class Renderer {
  public:
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  Renderer(Window& window, Device& device);
  ~Renderer();

  VkRenderPass getSwapchainRenderPass() const { return m_swapchain->getRenderPass(); }
  float getAspectRatio() const { return m_swapchain->extentAspectRatio(); }
  bool isFrameInProgress() const { return m_isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const
  {
    assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
    return m_commandBuffers[m_currentFrameIndex];
  }

  int getCurrentFrameIndex()
  {
    assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
    return m_currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();

  void beginSwapchainRenderPass(VkCommandBuffer cmd);
  void endSwapchainRenderPass(VkCommandBuffer cmd);

  private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapchain();

  Window& m_window;
  Device& m_device;
  std::unique_ptr<Swapchain> m_swapchain;
  std::vector<VkCommandBuffer> m_commandBuffers;

  uint32_t m_currentImageIndex;
  int m_currentFrameIndex { 0 };
  bool m_isFrameStarted = false;
};

} // namespace ve