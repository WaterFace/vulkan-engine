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
  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  Model(Device &device, const std::vector<Vertex> &vertices);
  ~Model();

  void bind(VkCommandBuffer cmd);
  void draw(VkCommandBuffer cmd);

private:
  Device &m_device;
  Buffer m_vertexBuffer;
  uint32_t m_vertexCount;

  void createVertexBuffers(const std::vector<Vertex> &vertices);
};

} // namespace ve