#include "simple_render_system.hpp"

#include "ve_descriptor_builder.hpp"
#include "ve_light.hpp"
#include "ve_pipeline_builder.hpp"
#include "ve_shader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>

namespace ve {

struct SimplePushConstantData {
  glm::mat4 mvp{1.0f};
};

struct InstanceData {
  glm::mat4 model;
  glm::mat4 normalRotation;
};

struct ObjectData {
  InstanceData objects[SimpleRenderSystem::MAX_INSTANCE_COUNT];
};

struct UniformData {
  // Camera data
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewproj;
  glm::vec3 cameraPosition;
};

struct LightData {
  uint32_t numLights;
  PointLight lights[SimpleRenderSystem::MAX_LIGHT_COUNT];
};

void loadScene(Scene &scene) {
  scene.addGameObject(glm::vec3(-1.0f, 0.0f, -2.5f), glm::vec3(0.0f), glm::vec3(0.5f), "smooth-monkey.glb");
  scene.addGameObject(glm::vec3(1.0f, 0.0f, -2.5f), glm::vec3(0.0f), glm::vec3(0.5f), "cube.gltf");
  scene.addGameObject(glm::vec3(0.0f, -2.0f, -2.5f), glm::vec3(0.0f), glm::vec3(0.5f), "uv-sphere.glb");

  scene.addLight({glm::vec3(2.0f, 0.0f, -1.5f), glm::vec3(0.8f, 0.8f, 0.8f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), 0.3f});
  scene.addLight({glm::vec3(-2.0f, 0.0f, -1.5f), glm::vec3(0.8f, 0.8f, 0.8f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), 0.3f});
  scene.addLight({glm::vec3(0.0f, 1.0f, -1.5f), glm::vec3(0.4f, 0.4f, 0.4f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), 0.3f});
}

SimpleRenderSystem::SimpleRenderSystem(Device &device, ModelLoader &modelLoader, VkRenderPass renderPass)
    : m_device{device}
    , m_modelLoader{modelLoader}
    , m_uniformBuffer{m_device.getAllocator()}
    , m_objectBuffer{m_device.getAllocator()}
    , m_lightBuffer{m_device.getAllocator()}
    , m_descriptorCache{device.device()}
    , m_descriptorAllocator{device.device()}
    , m_scene{modelLoader} {
  createPipeline(renderPass);

  loadScene(m_scene);
  m_scene.prepare();

  VkDeviceSize uniformBufferSize = m_device.padUniformBufferSize(sizeof(UniformData));
  std::cout << "Using a uniform buffer of size " << uniformBufferSize << std::endl;
  m_uniformBuffer.create(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0, VMA_MEMORY_USAGE_CPU_TO_GPU);
  m_uniformBuffer.mapMemory();

  VkDeviceSize objectBufferSize = MAX_INSTANCE_COUNT * sizeof(InstanceData);
  std::cout << "Using an object buffer of size " << objectBufferSize << std::endl;
  m_objectBuffer.create(objectBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0, VMA_MEMORY_USAGE_CPU_TO_GPU);
  m_objectBuffer.mapMemory();

  VkDeviceSize lightBufferSize = sizeof(uint32_t) + MAX_LIGHT_COUNT * sizeof(PointLight);
  std::cout << "Using a light buffer of size " << lightBufferSize << std::endl;
  m_lightBuffer.create(lightBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0, VMA_MEMORY_USAGE_CPU_TO_GPU);
  m_lightBuffer.mapMemory();

  VkDescriptorBufferInfo uniformBufferInfo{};
  uniformBufferInfo.buffer = m_uniformBuffer.buffer;
  uniformBufferInfo.offset = 0;
  uniformBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorBufferInfo objectBufferInfo{};
  objectBufferInfo.buffer = m_objectBuffer.buffer;
  objectBufferInfo.offset = 0;
  objectBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorBufferInfo lightBufferInfo{};
  lightBufferInfo.buffer = m_lightBuffer.buffer;
  lightBufferInfo.offset = 0;
  lightBufferInfo.range = VK_WHOLE_SIZE;

  VkDescriptorSet set0, set1;

  DescriptorBuilder::begin(&m_descriptorCache, &m_descriptorAllocator)
      .bindBuffer(
          0,
          &uniformBufferInfo,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
      .bindBuffer(
          1,
          &objectBufferInfo,
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
      .bindBuffer(
          2,
          &lightBufferInfo,
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
      .build(set0);

  DescriptorBuilder::begin(&m_descriptorCache, &m_descriptorAllocator)
      .bindSamplers(0, 1, &m_modelLoader.textureLoader().globalSamplerInfo(), VK_SHADER_STAGE_FRAGMENT_BIT)
      .bindImages(
          1,
          m_modelLoader.textureLoader().descriptorCount(),
          m_modelLoader.textureLoader().descriptorInfos().data(),
          VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(set1);
  // std::cout << m_modelLoader.textureLoader().descriptorCount() << std::endl;
  m_descriptorSets.push_back(set0);
  m_descriptorSets.push_back(set1);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  m_uniformBuffer.unmapMemory();
  m_objectBuffer.unmapMemory();
  m_lightBuffer.unmapMemory();
}

void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {

  auto vertShader = std::make_shared<ShaderStage>(m_device, "shaders/simple.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  auto fragShader = std::make_shared<ShaderStage>(m_device, "shaders/pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  PipelineBuilder builder(m_device);

  m_pipeline = builder.addShaderStage(vertShader)
                   .addShaderStage(fragShader)
                   .setSampleCount(m_device.getSampleCount())
                   .reflectLayout()
                   .setRenderPass(renderPass)
                   .setVertexInput(Model::Vertex::getBindingDescriptions(), Model::Vertex::getAttributeDescriptions())
                   .build();
}

void SimpleRenderSystem::renderGameObjects(
    VkCommandBuffer cmd,
    std::vector<GameObject> &gameObjects,
    const Camera &camera) {
  m_timer.update();
  m_pipeline->bind(cmd);

  m_modelLoader.bindBuffers(cmd);

  uint32_t totalInstanceCount = static_cast<uint32_t>(m_scene.gameObjects().size());
  assert(totalInstanceCount <= MAX_INSTANCE_COUNT && "Tried to draw more than the maximum number of instances");
  uint32_t uniformDataSize = sizeof(UniformData) + sizeof(InstanceData) * totalInstanceCount;

  UniformData *uniform = (UniformData *)m_uniformBuffer.data();
  uniform->view = camera.getView();
  uniform->proj = camera.getProjection();
  uniform->viewproj = uniform->proj * uniform->view;
  uniform->cameraPosition = camera.position();

  ObjectData *data = (ObjectData *)m_objectBuffer.data();

  LightData *lightData = (LightData *)m_lightBuffer.data();
  lightData->numLights = static_cast<uint32_t>(m_scene.lights().size());
  for (size_t i = 0; i < m_scene.lights().size(); i++) {
    lightData->lights[i] = m_scene.lights()[i];
  }

  for (uint32_t i = 0; i < totalInstanceCount; i++) {
    GameObject obj = m_scene.gameObjects()[i];

    data->objects[i].model = obj.transform.mat4();
    data->objects[i].normalRotation = glm::transpose(glm::inverse(data->objects[i].model));
  }
  vkCmdBindDescriptorSets(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipeline->layout(),
      0,
      static_cast<uint32_t>(m_descriptorSets.size()),
      m_descriptorSets.data(),
      0,
      nullptr);
  m_scene.draw(cmd);
}

} // namespace ve