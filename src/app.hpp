#pragma once

#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_renderer.hpp"
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

  Window m_window{WIDTH, HEIGHT, "First App"};
  Device m_device{m_window};
  Renderer m_renderer{m_window, m_device};

  std::vector<GameObject> m_gameObjects;
};

} // namespace ve