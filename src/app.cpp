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

void App::loadGameObjects() {
  // Mesh cubeModel = createCubeModel(m_device, {0.0f, 0.0f, 0.0f});
  // Mesh cubeModel = m_modelLoader.loadFromglTF("models/cube.gltf");
  Mesh monkeyModel = m_modelLoader.loadFromglTF("smooth-monkey.glb");
  Texture goldColor = m_modelLoader.textureLoader().loadFromFile("plastic/color.png");
  Texture goldDisplacement = m_modelLoader.textureLoader().loadFromFile("plastic/displacement.png");
  Texture goldMetal = m_modelLoader.textureLoader().loadFromFile("plastic/metal.png");
  Texture goldNormal = m_modelLoader.textureLoader().loadFromFile("plastic/normal.png");
  Texture goldRough = m_modelLoader.textureLoader().loadFromFile("plastic/rough.png");

  auto monkey = GameObject::createGameObject();
  monkey.mesh = monkeyModel;
  monkey.transform.translation = {0.0f, 0.0f, -2.5f};
  monkey.transform.scale = {0.5f, 0.5f, 0.5f};
  m_gameObjects.push_back(std::move(monkey));

  // auto cube = GameObject::createGameObject();
  // cube.model = cubeModel;
  // cube.transform.translation = {2.0f, 0.0f, -2.5f};
  // monkey.transform.scale = {0.5f, 0.5f, 0.5f};
  // m_gameObjects.push_back(std::move(cube));
}

} // namespace ve