#pragma once

#include "ve_texture.hpp"

#include <glm/glm.hpp>

namespace ve {

struct Material {
  float metallicFactor = 1.0f;
  float roughnessFactor = 1.0f;
  glm::vec4 baseColorFactor = glm::vec4(1.0f);
  glm::vec4 emissiveFactor = glm::vec4(1.0f);

  Texture baseColorTexture;
  Texture metallicRoughnessTexture;
  Texture normalTexture;
  Texture occlusionTexture;
  Texture emissiveTexture;
};

} // namespace ve