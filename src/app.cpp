#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <iostream>

namespace ve {

struct SimplePushConstantData {
  glm::mat2 transform{1.0f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

App::App() {
  loadGameObjects();
  createPipelineLayout();
  recreateSwapchain();
  createCommandBuffers();
}

App::~App() {
  vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void App::run() {
  while (!m_window.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(m_device.device());
}

void App::loadGameObjects() {
  std::vector<Model::Vertex> vertices{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  auto model = std::make_shared<Model>(m_device, vertices);
  auto triangle = GameObject::createGameObject();
  triangle.model = model;
  triangle.color = {0.1f, 0.8f, 0.1f};
  triangle.transform2D.translation.x = 0.2f;
  triangle.transform2D.scale = {2.0f, 0.5f};
  triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();

  m_gameObjects.push_back(std::move(triangle));
}

void App::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr,
                             &m_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }
}

void App::createPipeline() {
  PipelineConfigInfo pipelineConfig{};
  Pipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = m_swapchain->getRenderPass();
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  m_pipeline =
      std::make_unique<Pipeline>(m_device, "shaders/simple.vert.spv",
                                 "shaders/simple.frag.spv", pipelineConfig);
}

void App::recreateSwapchain() {
  auto extent = m_window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = m_window.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(m_device.device());

  if (m_swapchain == nullptr) {
    m_swapchain = std::make_unique<Swapchain>(m_device, extent);
  } else {
    m_swapchain =
        std::make_unique<Swapchain>(m_device, extent, std::move(m_swapchain));
    if (m_swapchain->imageCount() != m_commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline();
}

void App::createCommandBuffers() {
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

void App::freeCommandBuffers() {
  vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(),
                       static_cast<uint32_t>(m_commandBuffers.size()),
                       m_commandBuffers.data());
  m_commandBuffers.clear();
}

void App::recordCommandBuffer(int imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to begin command buffer");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_swapchain->getRenderPass();
  renderPassInfo.framebuffer = m_swapchain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swapchain->getSwapchainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(m_swapchain->getSwapchainExtent().width);
  viewport.height =
      static_cast<float>(m_swapchain->getSwapchainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, m_swapchain->getSwapchainExtent()};
  vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

  renderGameObjects(m_commandBuffers[imageIndex]);

  vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer");
  }
}

void App::renderGameObjects(VkCommandBuffer cmd) {
  m_pipeline->bind(cmd);

  for (auto &obj : m_gameObjects) {
    obj.transform2D.rotation =
        glm::mod(obj.transform2D.rotation + 0.01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset = obj.transform2D.translation;
    push.color = obj.color;
    push.transform = obj.transform2D.mat2();

    vkCmdPushConstants(cmd, m_pipelineLayout,
                       VK_SHADER_STAGE_FRAGMENT_BIT |
                           VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(SimplePushConstantData), &push);
    obj.model->bind(cmd);
    obj.model->draw(cmd);
  }
}

void App::drawFrame() {
  uint32_t imageIndex;
  auto result = m_swapchain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swapchain image");
  }

  recordCommandBuffer(imageIndex);
  result = m_swapchain->submitCommandBuffers(&m_commandBuffers[imageIndex],
                                             &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_window.wasWindowResized()) {
    m_window.resetWindowResizedFlag();
    recreateSwapchain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swapchain image");
  }
}

} // namespace ve