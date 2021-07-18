#include "ve_swapchain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace ve {

Swapchain::Swapchain(Device &deviceRef, VkExtent2D extent) : m_device{deviceRef}, m_windowExtent{extent} { init(); }
Swapchain::Swapchain(Device &deviceRef, VkExtent2D extent, std::shared_ptr<Swapchain> previous)
    : m_device{deviceRef}, m_windowExtent{extent}, m_oldSwapchain{previous} {
  init();

  m_oldSwapchain = nullptr;
}

void Swapchain::init() {
  createSwapchain();
  createImageViews();
  createRenderPass();
  createColorResources();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

Swapchain::~Swapchain() {
  for (auto imageView : m_swapchainImageViews) {
    vkDestroyImageView(m_device.device(), imageView, nullptr);
  }
  m_swapchainImageViews.clear();

  if (m_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
    m_swapchain = VK_NULL_HANDLE;
  }

  for (int i = 0; i < m_depthImages.size(); i++) {
    vkDestroyImageView(m_device.device(), m_depthImageViews[i], nullptr);
    vkDestroyImage(m_device.device(), m_depthImages[i], nullptr);
    vkFreeMemory(m_device.device(), m_depthImageMemories[i], nullptr);
  }

  for (int i = 0; i < m_colorImages.size(); i++) {
    vkDestroyImageView(m_device.device(), m_colorImageViews[i], nullptr);
    vkDestroyImage(m_device.device(), m_colorImages[i], nullptr);
    vkFreeMemory(m_device.device(), m_colorImageMemories[i], nullptr);
  }

  for (auto framebuffer : m_swapchainFramebuffers) {
    vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_device.device(), m_frameData[i].renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_device.device(), m_frameData[i].imageAvailableSemaphore, nullptr);
    vkDestroyFence(m_device.device(), m_frameData[i].inFlightFence, nullptr);
  }
}

VkResult Swapchain::acquireNextImage(uint32_t *imageIndex) {
  vkWaitForFences(m_device.device(), 1, &getCurrentFrame().inFlightFence, VK_TRUE,
                  std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(m_device.device(), m_swapchain, std::numeric_limits<uint64_t>::max(),
                                          getCurrentFrame().imageAvailableSemaphore, // must be a not signaled
                                                                                     // semaphore
                                          VK_NULL_HANDLE, imageIndex);

  return result;
}

VkResult Swapchain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
  if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  m_imagesInFlight[*imageIndex] = getCurrentFrame().inFlightFence;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {getCurrentFrame().imageAvailableSemaphore};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  VkSemaphore signalSemaphores[] = {getCurrentFrame().renderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(m_device.device(), 1, &getCurrentFrame().inFlightFence);
  if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, getCurrentFrame().inFlightFence) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapchains[] = {m_swapchain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;

  presentInfo.pImageIndices = imageIndex;

  auto result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void Swapchain::createSwapchain() {
  SwapchainSupportDetails swapchainSupport = m_device.getSwapchainSupport();

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_device.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = m_oldSwapchain == nullptr ? VK_NULL_HANDLE : m_oldSwapchain->m_swapchain;

  if (vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the
  // implementation is allowed to create a swap chain with more. That's why
  // we'll first query the final number of images with vkGetSwapchainImagesKHR,
  // then resize the container and finally call it again to retrieve the
  // handles.
  vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, nullptr);
  m_swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, m_swapchainImages.data());

  m_swapchainImageFormat = surfaceFormat.format;
  m_swapchainExtent = extent;
}

void Swapchain::createImageViews() {
  m_swapchainImageViews.resize(m_swapchainImages.size());
  for (size_t i = 0; i < m_swapchainImages.size(); i++) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_swapchainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_swapchainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void Swapchain::createRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = getSwapchainImageFormat();
  colorAttachment.samples = m_device.getSampleCount();
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = m_device.getSampleCount();
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = getSwapchainImageFormat();
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentResolveRef{};
  colorAttachmentResolveRef.attachment = 2;
  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;
  subpass.pResolveAttachments = &colorAttachmentResolveRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void Swapchain::createFramebuffers() {
  m_swapchainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 3> attachments = {m_colorImageViews[i], m_depthImageViews[i], m_swapchainImageViews[i]};

    VkExtent2D swapchainExtent = getSwapchainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapchainExtent.width;
    framebufferInfo.height = swapchainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void Swapchain::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  m_swapchainDepthFormat = depthFormat;
  VkExtent2D swapchainExtent = getSwapchainExtent();

  m_depthImages.resize(imageCount());
  m_depthImageMemories.resize(imageCount());
  m_depthImageViews.resize(imageCount());

  for (int i = 0; i < m_depthImages.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapchainExtent.width;
    imageInfo.extent.height = swapchainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = m_device.getSampleCount();
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i],
                                 m_depthImageMemories[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create depth image view!");
    }
  }
}

void Swapchain::createColorResources() {
  VkFormat colorFormat = m_swapchainImageFormat;
  VkExtent2D swapchainExtent = getSwapchainExtent();

  m_colorImages.resize(imageCount());
  m_colorImageMemories.resize(imageCount());
  m_colorImageViews.resize(imageCount());

  for (int i = 0; i < m_colorImages.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapchainExtent.width;
    imageInfo.extent.height = swapchainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = colorFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.samples = m_device.getSampleCount();
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorImages[i],
                                 m_colorImageMemories[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_colorImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = colorFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_colorImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create multisample image view!");
    }
  }
}

void Swapchain::createSyncObjects() {
  m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_frameData[i].imageAvailableSemaphore) !=
            VK_SUCCESS ||
        vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_frameData[i].renderFinishedSemaphore) !=
            VK_SUCCESS ||
        vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_frameData[i].inFlightFence) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = m_windowExtent;
    actualExtent.width =
        std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
        std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat Swapchain::findDepthFormat() {
  return m_device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace ve
