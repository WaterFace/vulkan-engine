#pragma once

#include "ve_camera.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_model_loader.hpp"
#include "ve_pipeline.hpp"

#include <memory>
#include <vector>

namespace ve {

class SimpleRenderSystem {
public:
  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  SimpleRenderSystem(Device &device, ModelLoader &modelLoader, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  void renderGameObjects(VkCommandBuffer cmd, std::vector<GameObject> &gameObjects, const Camera &camera);

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  Device &m_device;
  ModelLoader &m_modelLoader;

  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;
};

} // namespace ve