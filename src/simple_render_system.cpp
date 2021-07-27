#include "simple_render_system.hpp"

#include "ve_pipeline_builder.hpp"
#include "ve_shader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

namespace ve {

struct SimplePushConstantData {
  glm::mat4 mvp{1.0f};
  alignas(16) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem(Device &device, ModelLoader &modelLoader, VkRenderPass renderPass)
    : m_device{device}
    , m_modelLoader{modelLoader} {
  // createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {}

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
  auto fragShader = std::make_shared<ShaderStage>(m_device, "shaders/simple.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  PipelineBuilder builder(m_device);

  m_pipeline = builder.addShaderStage(vertShader)
                   .addShaderStage(fragShader)
                   .setSampleCount(m_device.getSampleCount())
                   //  .setLayout(m_pipelineLayout)
                   .reflectLayout()
                   .setRenderPass(renderPass)
                   .setVertexInput(Model::Vertex::getBindingDescriptions(), Model::Vertex::getAttributeDescriptions())
                   .build();
}

void SimpleRenderSystem::renderGameObjects(
    VkCommandBuffer cmd,
    std::vector<GameObject> &gameObjects,
    const Camera &camera) {
  m_pipeline->bind(cmd);

  m_modelLoader.bindBuffers(cmd);
  
  for (auto &obj : gameObjects) {
    obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.0001f * obj.getID(), glm::two_pi<float>());

    obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.0002f * obj.getID(), glm::two_pi<float>());

    SimplePushConstantData push{};
    push.mvp = camera.getProjection() * camera.getView() * obj.transform.mat4();

    vkCmdPushConstants(
        cmd,
        m_pipeline->layout(),
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    obj.model.draw(cmd);
  }
}

} // namespace ve