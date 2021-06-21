#pragma once

#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_pipeline.hpp"
#include "ve_swapchain.hpp"
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
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapchain();
  void recordCommandBuffer(int imageIndex);
  void renderGameObjects(VkCommandBuffer cmd);

  Window m_window{WIDTH, HEIGHT, "First App"};
  Device m_device{m_window};
  std::unique_ptr<Swapchain> m_swapchain;
  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;
  std::vector<VkCommandBuffer> m_commandBuffers;
  std::vector<GameObject> m_gameObjects;
};

} // namespace ve