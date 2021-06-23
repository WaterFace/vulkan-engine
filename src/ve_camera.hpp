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
  const glm::mat4 &getView() const { return m_view; };
  const glm::vec3 &forward() const { return m_forward; }
  const glm::vec3 &up() const { return m_up; }

  void rotate(float yaw, float pitch);
  void translate(glm::vec3 translation);

  void update(float dt);

private:
  float m_speed{2.0f};
  float m_sprintFactor{3.0f};
  MouseInput m_mouseInput{{}, true, true};
  KeyInput m_keyInput{
      {'W', 'S', 'A', 'D', GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ESCAPE}};
  glm::mat4 m_projection{1.0f};
  glm::mat4 m_view{1.0f};

  glm::vec3 m_position{};
  float m_yaw{0.0f};
  float m_pitch{0.0f};

  glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
  glm::vec3 m_up{0.0f, -1.0f, 0.0f};
  glm::vec3 m_left{-1.0f, 0.0f, 0.0f};

  void recalculateView();
  // TransformComponent transform;
};

} // namespace ve