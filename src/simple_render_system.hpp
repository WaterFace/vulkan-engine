#pragma once

#include "ve_camera.hpp"
#include "ve_descriptor_builder.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_mesh_loader.hpp"
#include "ve_pipeline.hpp"
#include "ve_scene.hpp"
#include "ve_timer.hpp"

#include <memory>
#include <vector>

namespace ve {

class SimpleRenderSystem {
public:
  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  SimpleRenderSystem(Device &device, MeshLoader &modelLoader, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  void renderGameObjects(VkCommandBuffer cmd, std::vector<GameObject> &gameObjects, const Camera &camera);

  static constexpr uint32_t MAX_INSTANCE_COUNT = 10000;
  static constexpr uint32_t MAX_LIGHT_COUNT = 100;
  static constexpr uint32_t MAX_MATERIAL_COUNT = 1000;

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  Device &m_device;
  MeshLoader &m_modelLoader;
  DescriptorLayoutCache m_descriptorCache;
  DescriptorAllocator m_descriptorAllocator;
  Timer m_timer{};
  Scene m_scene;

  std::vector<VkDescriptorSet> m_descriptorSets;
  Buffer m_uniformBuffer;
  Buffer m_primitiveBuffer;
  Buffer m_objectBuffer;
  Buffer m_lightBuffer;
  Buffer m_materialBuffer;

  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;
};

} // namespace ve