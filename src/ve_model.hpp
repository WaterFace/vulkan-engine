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

  struct Data {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
  };

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  Model(Device &device, const Model::Data &data);
  ~Model();

  void bind(VkCommandBuffer cmd);
  void draw(VkCommandBuffer cmd);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

  Device &m_device;
  Buffer m_vertexBuffer;
  Buffer m_indexBuffer;
  uint32_t m_vertexCount;
  uint32_t m_indexCount;
};

} // namespace ve