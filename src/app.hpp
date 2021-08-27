#pragma once

#include "ve_camera.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_mesh_loader.hpp"
#include "ve_renderer.hpp"
#include "ve_texture_loader.hpp"
#include "ve_timer.hpp"
#include "ve_window.hpp"

#include <memory>
#include <vector>

namespace ve {

class App {
public:
  static constexpr int WIDTH = 1024;
  static constexpr int HEIGHT = 768;

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  App();
  ~App();
  void run();

private:
  void loadGameObjects();
  Mesh createCubeModel(Device &device, glm::vec3 offset);

  Window m_window{WIDTH, HEIGHT, "First App"};
  Device m_device{m_window};
  Renderer m_renderer{m_window, m_device};
  MeshLoader m_modelLoader;

  Camera m_camera{};
  Timer m_timer{};

  std::vector<GameObject> m_gameObjects;
};

} // namespace ve