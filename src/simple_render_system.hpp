#pragma once

#include "ve_camera.hpp"
#include "ve_descriptor_builder.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_model_loader.hpp"
#include "ve_pipeline.hpp"
#include "ve_timer.hpp"

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

  static constexpr uint32_t MAX_INSTANCE_COUNT = 10000;
  static constexpr uint32_t MAX_LIGHT_COUNT = 100;

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  Device &m_device;
  ModelLoader &m_modelLoader;
  DescriptorLayoutCache m_descriptorCache;
  DescriptorAllocator m_descriptorAllocator;
  Timer m_timer{};

  VkDescriptorSet m_descriptorSet;
  Buffer m_uniformBuffer;
  Buffer m_objectBuffer;
  Buffer m_lightBuffer;

  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;
};

} // namespace ve