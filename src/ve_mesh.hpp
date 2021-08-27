#pragma once

#include "ve_buffer.hpp"
#include "ve_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace ve {

class Mesh {
public:
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  typedef uint32_t IndexType;
  static constexpr VkIndexType vulkanIndexType = VK_INDEX_TYPE_UINT32;

  struct Data {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
  };

  struct Primitive {
    int32_t material{-1};
    uint32_t vertexCount;
    uint32_t indexCount;
    Mesh::IndexType firstIndex;
    int32_t vertexOffset;
  };

  void draw(VkCommandBuffer cmd);

  bool operator==(const Mesh &other);
  uint32_t primitiveCount;
  uint32_t firstPrimitive;
};

} // namespace ve