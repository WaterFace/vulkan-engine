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
  addMaterial({});
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

Mesh MeshLoader::loadFromglTF(const std::string &filepath) {
  if (m_loadedMeshes.find(filepath) != m_loadedMeshes.end()) {
    std::cout << "MeshLoader: " << filepath << " is already loaded." << std::endl;
    return m_loadedMeshes[filepath];
  } else {
    glTF::Model model;
    model.loadFromFile(MODEL_PATH + filepath);

    // std::cout << "MeshLoader::loadFromglTF(): " << filepath << ":" << std::endl;
    // std::cout << "\tmodel.nodes.size(): " << model.nodes.size() << ":" << std::endl;

    size_t numberOfMeshes = 0;
    glTF::Mesh *firstMesh = nullptr;
    for (auto node : model.nodes) {
      if (node->mesh != nullptr) {
        if (firstMesh == nullptr) {
          firstMesh = node->mesh;
        }
        numberOfMeshes++;
      }
    }
    // std::cout << "firstMesh: " << firstMesh << std::endl;

    if (numberOfMeshes == 0) {
      std::cout << "MeshLoader::loadFromglTF(): ERROR: tried to load a gltf that doesn't contain any meshes"
                << std::endl;
      throw std::runtime_error("No meshes");
    }
    if (numberOfMeshes > 1) {
      std::cout << "MeshLoader::loadFromglTF(): WARNING: Trying to load glTF file with multiple meshes. Only the first "
                   "mesh will be "
                   "accessible, but all geometry in the file will be loaded into memory. One mesh per glTF file please."
                << std::endl;
    }

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

    // std::cout << "MeshLoader::loadFromglTF(): loading textures" << std::endl;

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

    // std::cout << "MeshLoader::loadFromglTF(): loading materials" << std::endl;

    std::vector<size_t> currentMeshMaterials{};

    size_t materialOffset = materials.size();
    for (auto material : model.materials) {
      Material newMaterial;
      newMaterial.baseColorFactor = material.baseColorFactor;
      newMaterial.emissiveFactor = material.emissiveFactor;
      newMaterial.metallicRoughnessFactor = glm::vec4(1.0f, material.roughnessFactor, material.metallicFactor, 1.0f);

      if (textures.size() == 0) {
        continue;
      }

      // std::cout << "MeshLoader::loadFromglTF(): no. of textures: " << textures.size() << std::endl;

      // std::cout << "MeshLoader::loadFromglTF(): base color index: " << material.baseColorTexture << std::endl;
      newMaterial.baseColorTexture = textures[material.baseColorTexture];
      // std::cout << "MeshLoader::loadFromglTF(): MR index: " << material.metallicRoughnessTexture << std::endl;
      newMaterial.metallicRoughnessTexture = textures[material.metallicRoughnessTexture];
      // std::cout << "MeshLoader::loadFromglTF(): emissive index: " << material.emissiveTexture << std::endl;
      newMaterial.emissiveTexture = textures[material.emissiveTexture];
      // std::cout << "MeshLoader::loadFromglTF(): normals index: " << material.normalTexture << std::endl;
      newMaterial.normalTexture = textures[material.normalTexture];
      // std::cout << "MeshLoader::loadFromglTF(): occlusion index: " << material.occlusionTexture << std::endl;
      newMaterial.occlusionTexture = textures[material.occlusionTexture];

      size_t matID = addMaterial(newMaterial);
      currentMeshMaterials.push_back(matID);
    }

    // Load primitives

    // std::cout << "MeshLoader::loadFromglTF(): loading primitives" << std::endl;

    std::cout << "MeshLoader::loadFromglTF(): # of materials in current mesh: " << currentMeshMaterials.size()
              << std::endl;

    Mesh newMesh;
    newMesh.primitiveCount = 0;
    newMesh.firstPrimitive = static_cast<uint32_t>(primitives.size());
    for (auto primitive : firstMesh->primitives) {
      Mesh::Primitive newPrimitive{};
      newPrimitive.firstIndex = static_cast<Mesh::IndexType>(primitive->firstIndex + m_currentIndexOffset);
      newPrimitive.indexCount = primitive->indexCount;
      newPrimitive.vertexCount = primitive->vertexCount;
      newPrimitive.vertexOffset = m_currentVertexOffset;
      if (primitive->material == -1) {
        std::cout << "MeshLoader::loadFromglTF(): primitive doesn't have a material, using the default" << std::endl;
        newPrimitive.material = 0;
      } else {
        std::cout << "MeshLoader::loadFromglTF(): adding primitive using material: " << primitive->material
                  << " (offset " << currentMeshMaterials[primitive->material] << ")" << std::endl;
        newPrimitive.material = currentMeshMaterials[primitive->material];
      }

      m_currentIndexOffset += newPrimitive.indexCount;
      m_currentVertexOffset += newPrimitive.vertexCount;
      primitives.push_back(newPrimitive);
      newMesh.primitiveCount++;
    }

    m_loadedMeshes[filepath] = newMesh;
    // std::cout << "MeshLoader::loadFromglTF(): finished loading " << filepath << std::endl;
    return newMesh;
  }
}

} // namespace ve