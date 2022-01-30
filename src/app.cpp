#include "app.hpp"

#include "ve_input.hpp"

#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <iostream>

namespace ve {

App::App()
    : m_modelLoader{m_device} {
  loadGameObjects();
  KeyInput::init(m_window.window());
  MouseInput::init(m_window.window());

  float aspect = m_renderer.getAspectRatio();
  m_camera.setPerspectiveProjection(45.0f, aspect, 0.01f, 100.0f);
}

App::~App() {}

void App::run() {
  SimpleRenderSystem simpleRenderSystem{m_device, m_modelLoader, m_renderer.getSwapchainRenderPass()};

  while (!m_window.shouldClose()) {
    glfwPollEvents();
    m_timer.update();

    m_camera.update(m_timer.dt());
    float aspect = m_renderer.getAspectRatio();
    if (glm::abs(m_camera.aspect() - aspect) > glm::epsilon<float>()) {
      std::cout << "Aspect ratio changed from " << m_camera.aspect() << " to " << aspect << std::endl;
      m_camera.setPerspectiveProjection(45.0f, aspect, 0.01f, 100.0f);
    }

    if (auto cmd = m_renderer.beginFrame()) {
      m_renderer.beginSwapchainRenderPass(cmd);
      simpleRenderSystem.renderGameObjects(cmd, m_gameObjects, m_camera);
      m_renderer.endSwapchainRenderPass(cmd);
      m_renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(m_device.device());
}

} // namespace ve