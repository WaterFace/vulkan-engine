#include "ve_model.hpp"

#include <cassert>
#include <cstring>

#include <iostream>

namespace ve {

Model::Model(Device &device, const std::vector<Vertex> &vertices)
    : m_device{device}, m_vertexBuffer{device.getAllocator()} {
  createVertexBuffers(vertices);
}
Model::~Model() {
  // The ve::Buffer should clean itself up when it falls out of scope
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
  m_vertexCount = static_cast<uint32_t>(vertices.size());

  assert(m_vertexCount >= 3 && "Need at least 3 vertices in a model");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

  m_vertexBuffer.create(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0);
  m_vertexBuffer.write((void *)vertices.data(), bufferSize);
}

void Model::bind(VkCommandBuffer cmd) {
  VkBuffer buffers[] = {m_vertexBuffer.buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
}

void Model::draw(VkCommandBuffer cmd) { vkCmdDraw(cmd, m_vertexCount, 1, 0, 0); }

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  return attributeDescriptions;
}

} // namespace ve