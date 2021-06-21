#include "ve_renderer.hpp"

#include <array>
#include <iostream>

namespace ve {

Renderer::Renderer(Window &window, Device &device)
    : m_window{window}, m_device{device} {
  recreateSwapchain();
  createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapchain() {
  auto extent = m_window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = m_window.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(m_device.device());

  if (m_swapchain == nullptr) {
    m_swapchain = std::make_unique<Swapchain>(m_device, extent);
  } else {
    std::shared_ptr<Swapchain> oldSwapchain = std::move(m_swapchain);
    m_swapchain = std::make_unique<Swapchain>(m_device, extent, oldSwapchain);

    if (!oldSwapchain->compareSwapFormats(*m_swapchain.get())) {
      throw std::runtime_error("Swapchain image or depth format has changed");
    }

    if (m_swapchain->imageCount() != m_commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }
}

void Renderer::createCommandBuffers() {
  m_commandBuffers.resize(m_swapchain->imageCount());
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_device.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

  if (vkAllocateCommandBuffers(m_device.device(), &allocInfo,
                               m_commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}

void Renderer::freeCommandBuffers() {
  vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(),
                       static_cast<uint32_t>(m_commandBuffers.size()),
                       m_commandBuffers.data());
  m_commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
  assert(!m_isFrameStarted &&
         "Can't call beginFrame() when a frame has already started");

  auto result = m_swapchain->acquireNextImage(&m_currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return nullptr;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swapchain image");
  }
  m_isFrameStarted = true;

  auto cmd = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin command buffer");
  }
  return cmd;
}
void Renderer::endFrame() {
  assert(m_isFrameStarted &&
         "Can't call endFrame() before a frame has started");
  auto cmd = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer");
  }
  auto result = m_swapchain->submitCommandBuffers(&cmd, &m_currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_window.wasWindowResized()) {
    m_window.resetWindowResizedFlag();
    recreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swapchain image");
  }
  m_isFrameStarted = false;
}

void Renderer::beginSwapchainRenderPass(VkCommandBuffer cmd) {
  assert(m_isFrameStarted &&
         "Can't call beginSwapchainRenderPass() before a frame has started");
  assert(cmd == getCurrentCommandBuffer() &&
         "Can't begin render pass on a command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_swapchain->getRenderPass();
  renderPassInfo.framebuffer = m_swapchain->getFrameBuffer(m_currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swapchain->getSwapchainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(m_swapchain->getSwapchainExtent().width);
  viewport.height =
      static_cast<float>(m_swapchain->getSwapchainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, m_swapchain->getSwapchainExtent()};
  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &scissor);
}
void Renderer::endSwapchainRenderPass(VkCommandBuffer cmd) {
  assert(m_isFrameStarted &&
         "Can't call endSwapchainRenderPass() if a frame is not in progress");
  assert(cmd == getCurrentCommandBuffer() &&
         "Can't end render pass on a command buffer from a different frame");

  vkCmdEndRenderPass(cmd);
}

} // namespace ve