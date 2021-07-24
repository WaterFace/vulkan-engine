#include "ve_model.hpp"

#include <cassert>
#include <cstring>

#include <iostream>

namespace ve {

Model::Model(Device &device, const Model::Data &data)
    : m_device{device}, m_vertexBuffer{device.getAllocator()}, m_indexBuffer{device.getAllocator()} {
  createVertexBuffers(data.vertices);
  createIndexBuffers(data.indices);
}
Model::~Model() {
  // The `ve::Buffer`s should clean themselves up when they falls out of scope
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
  m_vertexCount = static_cast<uint32_t>(vertices.size());

  assert(m_vertexCount >= 3 && "Need at least 3 vertices in a model");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

  Buffer stagingBuffer{m_device.getAllocator()};
  stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingBuffer.write((void *)vertices.data(), bufferSize);

  m_vertexBuffer.create(
      bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0, VMA_MEMORY_USAGE_GPU_ONLY);

  // TODO: Don't block while copying
  m_device.copyBuffer(stagingBuffer.buffer, m_vertexBuffer.buffer, bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
  m_indexCount = static_cast<uint32_t>(indices.size());

  assert(m_indexCount >= 3 && "Need at least 3 indices in a model");

  VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

  Buffer stagingBuffer{m_device.getAllocator()};
  stagingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingBuffer.write((void *)indices.data(), bufferSize);

  m_indexBuffer.create(
      bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0, VMA_MEMORY_USAGE_GPU_ONLY);

  // TODO: Don't block while copying
  m_device.copyBuffer(stagingBuffer.buffer, m_indexBuffer.buffer, bufferSize);
}

void Model::bind(VkCommandBuffer cmd) {
  VkBuffer buffers[] = {m_vertexBuffer.buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);

  vkCmdBindIndexBuffer(cmd, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::draw(VkCommandBuffer cmd) { vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0); }

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