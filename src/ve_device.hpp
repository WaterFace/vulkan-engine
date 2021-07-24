#pragma once

#include "ve_buffer.hpp"
#include "ve_window.hpp"

#include "vk_mem_alloc.h"

// std lib headers
#include <string>
#include <vector>

namespace ve {

struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device {
public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  Device(Window &window);
  ~Device();

  // Not copyable or movable
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device(Device &&) = delete;
  Device &operator=(Device &&) = delete;

  VkCommandPool getCommandPool() { return m_commandPool; }
  VkDevice device() { return m_device; }
  VkSurfaceKHR surface() { return m_surface; }
  VkQueue graphicsQueue() { return m_graphicsQueue; }
  VkQueue presentQueue() { return m_presentQueue; }
  VkSampleCountFlagBits getSampleCount() { return m_msaaSamples; }
  VmaAllocator getAllocator() { return m_allocator; }
  VkPhysicalDeviceProperties getPhysicalDeviceProperties() { return m_physicalDeviceProperties; }

  SwapchainSupportDetails getSwapchainSupport() { return querySwapchainSupport(m_physicalDevice); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBuffer(
      VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
  size_t padUniformBufferSize(size_t originalSize);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
      VkDeviceMemory &imageMemory);

  VkPhysicalDeviceProperties properties;

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
  VkSampleCountFlagBits getMaxUseableSampleCount();

  VkPhysicalDeviceProperties m_physicalDeviceProperties;

  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_debugMessenger;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  Window &m_window;
  VkCommandPool m_commandPool;

  VkDevice m_device;
  VkSurfaceKHR m_surface;
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;

  VmaAllocator m_allocator;

  VkSampleCountFlagBits m_msaaSamples{VK_SAMPLE_COUNT_1_BIT};

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace ve