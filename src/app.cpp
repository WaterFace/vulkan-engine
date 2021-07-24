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

App::App() {
  loadGameObjects();
  KeyInput::init(m_window.window());
  MouseInput::init(m_window.window());

  float aspect = m_renderer.getAspectRatio();
  m_camera.setPerspectiveProjection(45.0f, aspect, 0.01f, 100.0f);
}

App::~App() {}

void App::run() {
  SimpleRenderSystem simpleRenderSystem{m_device, m_renderer.getSwapchainRenderPass()};

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

std::unique_ptr<Model> createCubeModel(Device &device, glm::vec3 offset) {
  Model::Data data{};
  data.vertices = {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
  };
  for (auto &v : data.vertices) {
    v.position += offset;
  }

  data.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                  12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

  return std::make_unique<Model>(device, data);
}

void App::loadGameObjects() {
  std::shared_ptr<Model> cubeModel = createCubeModel(m_device, {0.0f, 0.0f, 0.0f});

  for (int i = 0; i < 100; i++) {
    auto cube = GameObject::createGameObject();
    cube.model = cubeModel;
    cube.transform.translation = {(i / 10) + 0.0f, (i % 10) + 0.0f, -2.5f};
    cube.transform.scale = {0.5f, 0.5f, 0.5f};
    m_gameObjects.push_back(std::move(cube));
  }
}

} // namespace ve