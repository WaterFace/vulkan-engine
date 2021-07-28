#include "ve_model_loader.hpp"

#include "ve_gltf_loader.hpp"

#include <iostream>

namespace ve {

ModelLoader::ModelLoader(Device &device)
    : m_device{device}
    , m_invalidBuffers{true} {
  m_bigVertexBuffer = std::make_unique<Buffer>(m_device.getAllocator());
  m_bigIndexBuffer = std::make_unique<Buffer>(m_device.getAllocator());
  m_bigVertexBuffer->create(
      INITIAL_BUFFER_SIZE,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      0,
      VMA_MEMORY_USAGE_GPU_ONLY);
  m_bigIndexBuffer->create(
      INITIAL_BUFFER_SIZE,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      0,
      VMA_MEMORY_USAGE_GPU_ONLY);
}

ModelLoader::~ModelLoader() {}

void ModelLoader::growVertexBuffer() {
  std::cout << "ModelLoader: grew vertex buffer. New size: " << m_currentVertexBufferSize * 2 << std::endl;
  auto newBuffer = std::make_unique<Buffer>(m_device.getAllocator());
  newBuffer->create(
      m_currentVertexBufferSize * 2,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      0,
      VMA_MEMORY_USAGE_GPU_ONLY);
  m_device.copyBuffer(m_bigVertexBuffer->buffer, newBuffer->buffer, m_currentVertexBufferSize);

  m_currentVertexBufferSize *= 2;
  m_bigVertexBuffer = std::move(newBuffer);
  m_invalidBuffers = true;
}

void ModelLoader::growIndexBuffer() {
  std::cout << "ModelLoader: grew index buffer. New size: " << m_currentIndexBufferSize * 2 << std::endl;
  auto newBuffer = std::make_unique<Buffer>(m_device.getAllocator());
  newBuffer->create(
      m_currentIndexBufferSize * 2,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      0,
      VMA_MEMORY_USAGE_GPU_ONLY);
  m_device.copyBuffer(m_bigIndexBuffer->buffer, newBuffer->buffer, m_currentIndexBufferSize);

  m_currentIndexBufferSize *= 2;
  m_bigIndexBuffer = std::move(newBuffer);
  m_invalidBuffers = true;
}

void ModelLoader::bindBuffers(VkCommandBuffer cmd) {
  VkBuffer buffers[] = {m_bigVertexBuffer->buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);

  vkCmdBindIndexBuffer(cmd, m_bigIndexBuffer->buffer, 0, Model::vulkanIndexType);

  m_invalidBuffers = false;
}

Model ModelLoader::load(const Model::Data &data) {
  VkDeviceSize vertexBufferSize = data.vertices.size() * sizeof(Model::Vertex);
  VkDeviceSize indexBufferSize = data.indices.size() * sizeof(Model::IndexType);
  // VkDeviceSize stagingBufferSize = vertexBufferSize + indexBufferSize;

  Buffer stagingVertexBuffer{m_device.getAllocator()};
  stagingVertexBuffer.create(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingVertexBuffer.write((void *)data.vertices.data(), vertexBufferSize);

  Buffer stagingIndexBuffer{m_device.getAllocator()};
  stagingIndexBuffer.create(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
  stagingIndexBuffer.write((void *)data.indices.data(), indexBufferSize);

  while (m_currentVertexBufferSize - m_currentVertexOffset < vertexBufferSize) {
    growVertexBuffer();
  }

  m_device
      .copyBuffer(stagingVertexBuffer.buffer, m_bigVertexBuffer->buffer, vertexBufferSize, 0, m_currentVertexOffset);

  while (m_currentIndexBufferSize - m_currentIndexOffset < indexBufferSize) {
    growIndexBuffer();
  }

  m_device.copyBuffer(stagingIndexBuffer.buffer, m_bigIndexBuffer->buffer, indexBufferSize, 0, m_currentIndexOffset);

  Model model{
      static_cast<uint32_t>(data.vertices.size()),
      static_cast<uint32_t>(data.indices.size()),
      m_currentIndexOffset,
      static_cast<int32_t>(m_currentVertexOffset)};

  m_currentVertexOffset += vertexBufferSize;
  m_currentIndexOffset += indexBufferSize;

  return model;
}

Model ModelLoader::loadFromglTF(const std::string &filepath) {
  glTF::Model model;
  model.loadFromFile(filepath, &m_device);

  return load(model.data);
}

} // namespace ve