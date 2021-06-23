#pragma once

#include "ve_game_object.hpp"
#include "ve_input.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ve {

class Camera {
public:
  void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
  void setPerspectiveProjection(float vfov, float aspect, float near, float far);

  const glm::mat4 &getProjection() const { return m_projection; }
  const glm::mat4 getView() const;

  void update(float dt);

private:
  float m_speed{1.0f};
  KeyInput keyInput{{'W', 'S', 'A', 'D'}};
  glm::mat4 m_projection{1.0f};
  TransformComponent transform;
};

} // namespace ve