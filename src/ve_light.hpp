#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace ve {

struct PointLight {
  alignas(16) glm::vec3 position;

  alignas(16) glm::vec3 diffuseColor;
  alignas(4) float diffusePower;

  alignas(16) glm::vec3 specularColor;
  alignas(4) float specularPower;
};

}