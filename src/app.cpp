#include "app.hpp"

#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <iostream>

namespace ve {

App::App() { loadGameObjects(); }

App::~App() {}

void App::run() {
  SimpleRenderSystem simpleRenderSystem{m_device,
                                        m_renderer.getSwapchainRenderPass()};
  while (!m_window.shouldClose()) {
    glfwPollEvents();

    if (auto cmd = m_renderer.beginFrame()) {
      m_renderer.beginSwapchainRenderPass(cmd);
      simpleRenderSystem.renderGameObjects(cmd, m_gameObjects);
      m_renderer.endSwapchainRenderPass(cmd);
      m_renderer.endFrame();
    }
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

} // namespace ve