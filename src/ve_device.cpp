#include "ve_device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace ve {

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// class member functions
Device::Device(Window &window)
    : m_window{window} {
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createAllocator();
  createCommandPool();
}

Device::~Device() {
  vmaDestroyAllocator(m_allocator);
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
  vkDestroyDevice(m_device, nullptr);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}

void Device::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LittleVulkanEngine App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  hasGflwRequiredInstanceExtensions();
}

void Device::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::cout << "Device count: " << deviceCount << std::endl;
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      m_physicalDevice = device;
      m_msaaSamples = getMaxUseableSampleCount();
      break;
    }
  }

  if (m_physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
  m_physicalDeviceProperties = properties;
  std::cout << "physical device: " << getPhysicalDeviceProperties().deviceName << std::endl;
}

void Device::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // might not really be necessary anymore because device specific validation
  // layers have been deprecated
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
}

void Device::createAllocator() {
  VmaAllocatorCreateInfo info{};
  info.device = m_device;
  info.instance = m_instance;
  info.physicalDevice = m_physicalDevice;

  vmaCreateAllocator(&info, &m_allocator);
}

void Device::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void Device::createSurface() { m_window.createWindowSurface(m_instance, &m_surface); }

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapchainAdequate = false;
  if (extensionsSupported) {
    SwapchainSupportDetails swapChainSupport = querySwapchainSupport(device);
    swapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapchainAdequate && supportedFeatures.samplerAnisotropy;
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr; // Optional
}

void Device::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

bool Device::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

std::vector<const char *> Device::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
    available.insert(extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapchainSupportDetails Device::querySwapchainSupport(VkPhysicalDevice device) {
  SwapchainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
  }
  return details;
}

VkSampleCountFlagBits Device::getMaxUseableSampleCount() {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

  VkSampleCountFlags counts =
      properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }

  return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Device::findSupportedFormat(
    const std::vector<VkFormat> &candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer Device::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_graphicsQueue);

  vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  copyBuffer(srcBuffer, dstBuffer, size, 0, 0);
}

void Device::copyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size,
    VkDeviceSize srcOffset,
    VkDeviceSize dstOffset) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}

void Device::imageLayoutTransition(
    VkImage image,
    uint32_t layerCount,
    uint32_t levelCount,
    VkImageAspectFlags aspectMask,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask) {
  VkCommandBuffer cmd = beginSingleTimeCommands();

  VkImageSubresourceRange range{};
  range.aspectMask = aspectMask;
  range.layerCount = layerCount;
  range.baseArrayLayer = 0;
  range.levelCount = levelCount;
  range.baseMipLevel = 0;

  VkImageMemoryBarrier imageBarrierToTransfer{};
  imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageBarrierToTransfer.pNext = nullptr;

  imageBarrierToTransfer.oldLayout = oldLayout;
  imageBarrierToTransfer.newLayout = newLayout;
  imageBarrierToTransfer.image = image;
  imageBarrierToTransfer.subresourceRange = range;

  imageBarrierToTransfer.srcAccessMask = srcAccessMask;
  imageBarrierToTransfer.dstAccessMask = dstAccessMask;

  vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

  endSingleTimeCommands(cmd);
}

size_t Device::padUniformBufferSize(size_t originalSize) {
  // from https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer

  // Calculate required alignment based on minimum device offset alignment
  size_t minUboAlignment = getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
  size_t alignedSize = originalSize;
  if (minUboAlignment > 0) {
    alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }
  return alignedSize;
}

void Device::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) {
  if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(m_device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(m_device, image, imageMemory, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to bind image memory!");
  }
}

} // namespace ve