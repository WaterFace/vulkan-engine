#include "ve_mesh_loader.hpp"

#include "ve_gltf_loader.hpp"

#include <iostream>

namespace ve {

const std::string MeshLoader::MODEL_PATH = "models/";

MeshLoader::MeshLoader(Device &device)
    : m_device{device}
    , m_invalidBuffers{true}
    , m_textureLoader{device} {
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

  // add default empty material at index 0
  m_materials.push_back(Material{});
}

MeshLoader::~MeshLoader() {}

void MeshLoader::growVertexBuffer() {
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

void MeshLoader::growIndexBuffer() {
  std::cout << "MeshLoader: grew index buffer. New size: " << m_currentIndexBufferSize * 2 << std::endl;
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

void MeshLoader::bindBuffers(VkCommandBuffer cmd) {
  VkBuffer buffers[] = {m_bigVertexBuffer->buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);

  vkCmdBindIndexBuffer(cmd, m_bigIndexBuffer->buffer, 0, Mesh::vulkanIndexType);

  m_invalidBuffers = false;
}

// Mesh MeshLoader::load(const Mesh::Data &data) {
//   VkDeviceSize vertexBufferSize = data.vertices.size() * sizeof(Mesh::Vertex);
//   VkDeviceSize indexBufferSize = data.indices.size() * sizeof(Mesh::IndexType);
//   // VkDeviceSize stagingBufferSize = vertexBufferSize + indexBufferSize;

//   Buffer stagingVertexBuffer{m_device.getAllocator()};
//   stagingVertexBuffer.create(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
//   stagingVertexBuffer.write((void *)data.vertices.data(), vertexBufferSize);

//   Buffer stagingIndexBuffer{m_device.getAllocator()};
//   stagingIndexBuffer.create(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
//   stagingIndexBuffer.write((void *)data.indices.data(), indexBufferSize);

//   while (m_currentVertexBufferSize - m_currentVertexOffset < vertexBufferSize) {
//     growVertexBuffer();
//   }

//   m_device.copyBuffer(
//       stagingVertexBuffer.buffer,
//       m_bigVertexBuffer->buffer,
//       vertexBufferSize,
//       0,
//       m_currentVertexOffset * sizeof(Mesh::Vertex));

//   while (m_currentIndexBufferSize - m_currentIndexOffset < indexBufferSize) {
//     growIndexBuffer();
//   }

//   m_device.copyBuffer(
//       stagingIndexBuffer.buffer,
//       m_bigIndexBuffer->buffer,
//       indexBufferSize,
//       0,
//       m_currentIndexOffset * sizeof(Mesh::IndexType));

//   Mesh model{
//       static_cast<uint32_t>(data.vertices.size()),
//       static_cast<uint32_t>(data.indices.size()),
//       m_currentIndexOffset,
//       static_cast<int32_t>(m_currentVertexOffset)};

//   m_currentVertexOffset += data.vertices.size();
//   m_currentIndexOffset += data.indices.size();

//   return model;
// }

Mesh MeshLoader::loadFromglTF(const std::string &filepath) {

  if (m_loadedModels.find(filepath) == m_loadedModels.end()) {
    glTF::Model model;
    model.loadFromFile(MODEL_PATH + filepath);

    size_t numberOfMeshes = 0;
    glTF::Mesh *firstMesh;
    for (auto node : model.nodes) {
      if (node->mesh != nullptr) {
        if (firstMesh == nullptr) {
          firstMesh = node->mesh;
        }
        numberOfMeshes++;
      }
    }

    std::cout << "MeshLoader: WARNING: Trying to load glTF file with multiple meshes. Only the first mesh will be "
                 "accessible, but all geometry in the file will be loaded into memory. One mesh per glTF file please."
              << std::endl;

    // upload geometry data to GPU
    VkDeviceSize vertexBufferSize = model.vertexBuffer.size() * sizeof(Mesh::Vertex);
    VkDeviceSize indexBufferSize = model.indexBuffer.size() * sizeof(Mesh::IndexType);

    Buffer stagingVertexBuffer{m_device.getAllocator()};
    stagingVertexBuffer.create(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingVertexBuffer.write((void *)model.vertexBuffer.data(), vertexBufferSize);

    Buffer stagingIndexBuffer{m_device.getAllocator()};
    stagingIndexBuffer.create(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingIndexBuffer.write((void *)model.indexBuffer.data(), indexBufferSize);

    while (m_currentVertexBufferSize - m_currentVertexOffset < vertexBufferSize) {
      growVertexBuffer();
    }

    m_device.copyBuffer(
        stagingVertexBuffer.buffer,
        m_bigVertexBuffer->buffer,
        vertexBufferSize,
        0,
        m_currentVertexOffset * sizeof(Mesh::Vertex));

    while (m_currentIndexBufferSize - m_currentIndexOffset < indexBufferSize) {
      growIndexBuffer();
    }

    m_device.copyBuffer(
        stagingIndexBuffer.buffer,
        m_bigIndexBuffer->buffer,
        indexBufferSize,
        0,
        m_currentIndexOffset * sizeof(Mesh::IndexType));

    // load textures

    std::vector<Texture> textures;
    for (auto texture : model.textures) {
      Texture newTexture;
      if (texture.isExternalTexture) {
        newTexture = m_textureLoader.loadFromFile(texture.texturePath);
      } else {
        newTexture = m_textureLoader.loadFromData(texture.rawData.data(), texture.width, texture.height);
      }
      textures.push_back(newTexture);
    }

    // load materials

    size_t materialOffset = m_materials.size();
    for (auto material : model.materials) {
      Material newMaterial;
      newMaterial.baseColorFactor = material.baseColorFactor;
      newMaterial.emissiveFactor = material.emissiveFactor;
      newMaterial.metallicFactor = material.metallicFactor;
      newMaterial.roughnessFactor = material.roughnessFactor;

      newMaterial.baseColorTexture = textures[material.baseColorTexture];
      newMaterial.metallicRoughnessTexture = textures[material.metallicRoughnessTexture];
      newMaterial.emissiveTexture = textures[material.emissiveTexture];
      newMaterial.normalTexture = textures[material.normalTexture];
      newMaterial.occlusionTexture = textures[material.occlusionTexture];

      m_materials.push_back(newMaterial);
    }

    Mesh newMesh;
    newMesh.primitiveCount = 0;
    newMesh.firstPrimitive = static_cast<uint32_t>(m_primitives.size());
    for (auto primitive : firstMesh->primitives) {
      Mesh::Primitive newPrimitive;
      newPrimitive.firstIndex = static_cast<Mesh::IndexType>(primitive->firstIndex) + m_currentIndexOffset;
      newPrimitive.indexCount = primitive->indexCount;
      newPrimitive.vertexCount = primitive->vertexCount;
      newPrimitive.vertexOffset = m_currentVertexOffset;
      newPrimitive.material = primitive->material;

      m_primitives.push_back(newPrimitive);
      newMesh.primitiveCount++;
    }

    return newMesh;
  } else {
    return m_loadedModels[filepath];
  }
}

} // namespace ve