#pragma once

#include "ve_buffer.hpp"
#include "ve_device.hpp"
#include "ve_model.hpp"

#include <memory>
#include <string>

namespace ve {

// struct LoadedModel {
//   uint32_t indexCount;
//   uint32_t firstIndex;
//   int32_t vertexOffset;
// };

class ModelLoader {
public:
  ModelLoader(Device &device);
  ~ModelLoader();

  void bindBuffers(VkCommandBuffer cmd);

  bool invalidBuffers() { return m_invalidBuffers; }

  static const std::string MODEL_PATH;
  Model load(const Model::Data &data);
  Model loadFromglTF(const std::string &filepath);

private:
  void growVertexBuffer();
  void growIndexBuffer();

  bool m_invalidBuffers{true};

  static constexpr VkDeviceSize INITIAL_BUFFER_SIZE = 1000;
  Device &m_device;

  std::unique_ptr<Buffer> m_bigVertexBuffer;
  uint32_t m_currentVertexBufferSize{INITIAL_BUFFER_SIZE};
  uint32_t m_currentVertexOffset{0};

  std::unique_ptr<Buffer> m_bigIndexBuffer;
  uint32_t m_currentIndexBufferSize{INITIAL_BUFFER_SIZE};
  uint32_t m_currentIndexOffset{0};
};

} // namespace ve
