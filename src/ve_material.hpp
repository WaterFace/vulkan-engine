#pragma once

#include "ve_texture.hpp"

#include <glm/glm.hpp>

namespace ve {

class Material {
  glm::vec4 baseColorFactor = glm::vec4(1.0f);
  float metallicFactor = 1.0f;
  float roughnessFactor = 1.0f;
  glm::vec4 emissiveFactor = glm::vec4(1.0f);

  struct TextureSet {
    Texture albedo{0};
    Texture metallic{0};
    Texture roughness{0};
    Texture ao{0};
    Texture emissive{0};
  };

  TextureSet textures;
};

} // namespace ve