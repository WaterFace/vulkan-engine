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

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  typedef uint32_t IndexType;
  static constexpr VkIndexType vulkanIndexType = VK_INDEX_TYPE_UINT32;

  struct Data {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
  };

  Model(uint32_t vertexCount, uint32_t indexCount, IndexType firstIndex, int32_t vertexOffset)
      : m_vertexCount{vertexCount}
      , m_indexCount{indexCount}
      , m_firstIndex{firstIndex}
      , m_vertexOffset{vertexOffset} {}
  Model()
      : m_vertexCount{0}
      , m_indexCount{0}
      , m_firstIndex{0}
      , m_vertexOffset{0} {}
  ~Model(){};

  void draw(VkCommandBuffer cmd) { vkCmdDrawIndexed(cmd, m_indexCount, 1, m_firstIndex, m_vertexOffset, 0); };

private:
  uint32_t m_vertexCount;
  uint32_t m_indexCount;
  IndexType m_firstIndex;
  int32_t m_vertexOffset;
};

} // namespace ve