#include "ve_camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cassert>
#include <iostream>
#include <limits>

namespace ve {

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
  m_projection = glm::ortho(left, right, bottom, top);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  m_projection = glm::perspective(fovy, aspect, near, far);
}

void Camera::translate(glm::vec3 translation) {
  m_position += translation;
  recalculateView();
}

void Camera::rotate(float yaw, float pitch) {
  m_yaw += yaw;
  m_pitch -= pitch;
  m_pitch = glm::clamp(m_pitch, -glm::half_pi<float>() * 0.98f, glm::half_pi<float>() * 0.98f);
  recalculateView();
  m_forward = glm::rotate(m_forward, yaw, m_up);
  m_forward = glm::rotate(m_forward, -pitch, m_left);
  m_left = glm::cross(m_forward, m_up);
}

void Camera::recalculateView() {
  glm::mat4 m{1.0f};

  m = glm::translate(m, m_position);
  m = glm::rotate(m, m_yaw, {0.0f, -1.0f, 0.0f});
  m = glm::rotate(m, m_pitch, {-1.0f, 0.0f, 0.0f});
  // m = glm::rotate(m, m_roll, {0.0f, 0.0f, -1.0f});

  m_view = glm::inverse(m);
}

void Camera::update(float dt) {
  float speed = m_speed;

  if (m_keyInput.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
    speed = m_speed * m_sprintFactor;
  } else {
    speed = m_speed;
  }

  if (m_keyInput.isKeyDown('W')) {
    translate(m_forward * speed * dt);
  }
  if (m_keyInput.isKeyDown('S')) {
    translate(-m_forward * speed * dt);
  }
  if (m_keyInput.isKeyDown('A')) {
    translate(m_left * speed * dt);
  }
  if (m_keyInput.isKeyDown('D')) {
    translate(-m_left * speed * dt);
  }
  if (m_keyInput.isKeyDown(GLFW_KEY_SPACE)) {
    translate(m_up * speed * dt);
  }
  if (m_keyInput.isKeyDown(GLFW_KEY_LEFT_CONTROL)) {
    translate(-m_up * speed * dt);
  }

  static bool previousState = false;
  bool currentState = m_keyInput.isKeyDown(GLFW_KEY_ESCAPE);
  if (currentState && !previousState) {
    if (MouseInput::getInputMode() == MouseInput::InputMode::Raw) {
      MouseInput::setInputMode(MouseInput::InputMode::Absolute);
    } else if (MouseInput::getInputMode() == MouseInput::InputMode::Absolute) {
      MouseInput::setInputMode(MouseInput::InputMode::Raw);
    }
  }
  previousState = currentState;

  static double prevX = m_mouseInput.x();
  static double prevY = m_mouseInput.y();
  double x = m_mouseInput.x() - prevX;
  double y = m_mouseInput.y() - prevY;
  prevX = m_mouseInput.x();
  prevY = m_mouseInput.y();

  constexpr float SENSITIVITY = 0.005;

  if (MouseInput::getInputMode() == MouseInput::InputMode::Raw) {
    rotate(x * SENSITIVITY, y * SENSITIVITY);
  }
}

} // namespace ve