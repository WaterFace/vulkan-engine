#pragma once

#include "ve_texture.hpp"

#include <glm/glm.hpp>

namespace ve {

struct Material {
  glm::vec4 metallicRoughnessFactor = glm::vec4(1.0f);
  glm::vec4 baseColorFactor = glm::vec4(1.0f);
  glm::vec4 emissiveFactor = glm::vec4(1.0f);

  Texture baseColorTexture;
  Texture metallicRoughnessTexture;
  Texture normalTexture;
  Texture occlusionTexture;
  alignas(16) Texture emissiveTexture;
};

} // namespace ve