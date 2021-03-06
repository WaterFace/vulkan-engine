#pragma once

#include "ve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace ve {

struct FrameData {
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;
};

class Swapchain {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  Swapchain(Device &deviceRef, VkExtent2D windowExtent);
  Swapchain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<Swapchain> previous);
  ~Swapchain();

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return m_swapchainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return m_renderPass; }
  VkImageView getImageView(int index) { return m_swapchainImageViews[index]; }
  size_t imageCount() { return m_swapchainImages.size(); }
  FrameData &getCurrentFrame() { return m_frameData[m_currentFrame % MAX_FRAMES_IN_FLIGHT]; }
  VkFormat getSwapchainImageFormat() { return m_swapchainImageFormat; }
  VkExtent2D getSwapchainExtent() { return m_swapchainExtent; }
  uint32_t width() { return m_swapchainExtent.width; }
  uint32_t height() { return m_swapchainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool compareSwapFormats(const Swapchain &swapchain) {
    return swapchain.m_swapchainImageFormat == m_swapchainImageFormat &&
           swapchain.m_swapchainDepthFormat == m_swapchainDepthFormat;
  }

private:
  void init();
  void createSwapchain();
  void createImageViews();
  void createColorResources();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat m_swapchainImageFormat;
  VkFormat m_swapchainDepthFormat;
  VkExtent2D m_swapchainExtent;

  std::vector<VkFramebuffer> m_swapchainFramebuffers;
  VkRenderPass m_renderPass;

  // per-framebuffer data
  std::vector<VkImage> m_colorImages;
  std::vector<VkDeviceMemory> m_colorImageMemories;
  std::vector<VkImageView> m_colorImageViews;
  std::vector<VkImage> m_depthImages;
  std::vector<VkDeviceMemory> m_depthImageMemories;
  std::vector<VkImageView> m_depthImageViews;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;
  std::vector<VkFence> m_imagesInFlight;

  // per-frame data
  FrameData m_frameData[MAX_FRAMES_IN_FLIGHT];

  Device &m_device;
  VkExtent2D m_windowExtent;

  VkSwapchainKHR m_swapchain;
  std::shared_ptr<Swapchain> m_oldSwapchain;

  size_t m_currentFrame = 0;
};

} // namespace ve
