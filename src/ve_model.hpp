#pragma once

#include "ve_buffer.hpp"
#include "ve_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace ve {

class Model {
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

  Model(uint32_t vCount, uint32_t iCount, IndexType first, int32_t offset)
      : vertexCount{vCount}
      , indexCount{iCount}
      , firstIndex{first}
      , vertexOffset{offset} {}
  Model()
      : vertexCount{0}
      , indexCount{0}
      , firstIndex{0}
      , vertexOffset{0} {}
  ~Model(){};

  void draw(VkCommandBuffer cmd) { vkCmdDrawIndexed(cmd, indexCount, 1, firstIndex, vertexOffset, 0); };

  uint32_t vertexCount;
  uint32_t indexCount;
  IndexType firstIndex;
  int32_t vertexOffset;
};

} // namespace ve