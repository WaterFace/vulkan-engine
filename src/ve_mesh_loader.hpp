#pragma once

#include "ve_buffer.hpp"
#include "ve_device.hpp"
#include "ve_material.hpp"
#include "ve_mesh.hpp"
#include "ve_texture_loader.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace ve {

class MeshLoader {
public:
  MeshLoader(Device &device);
  ~MeshLoader();

  void bindBuffers(VkCommandBuffer cmd);

  bool invalidBuffers() { return m_invalidBuffers; }

  TextureLoader &textureLoader() { return m_textureLoader; }

  Mesh::Primitive &getPrimitive(size_t i) { return primitives[i]; }
  Material &getMaterial(size_t i) { return materials[i]; }

  size_t addMaterial(Material mat) {
    std::cout << "adding material with texture indices " << mat.baseColorTexture.id << ", " << mat.normalTexture.id
              << ", " << mat.metallicRoughnessTexture.id << std::endl;
    materials.push_back(mat);
    return materials.size() - 1;
  };

  static const std::string MODEL_PATH;
  // Mesh loadPrimitive(const Mesh::Data &data);
  Mesh loadFromglTF(const std::string &filepath);

  std::vector<Material> materials;
  std::vector<Mesh::Primitive> primitives;

private:
  void growVertexBuffer();
  void growIndexBuffer();

  bool m_invalidBuffers{true};

  TextureLoader m_textureLoader;

  static constexpr VkDeviceSize INITIAL_BUFFER_SIZE = 1000;
  Device &m_device;

  std::unordered_map<std::string, Mesh> m_loadedMeshes;

  std::unique_ptr<Buffer> m_bigVertexBuffer;
  uint32_t m_currentVertexBufferSize{INITIAL_BUFFER_SIZE};
  uint32_t m_currentVertexOffset{0};

  std::unique_ptr<Buffer> m_bigIndexBuffer;
  uint32_t m_currentIndexBufferSize{INITIAL_BUFFER_SIZE};
  uint32_t m_currentIndexOffset{0};
};

} // namespace ve
