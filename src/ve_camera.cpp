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
  m_aspect = (right - left) / (top - bottom);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  m_projection = glm::perspective(fovy, aspect, near, far);
  m_aspect = aspect;
}

void Camera::translate(glm::vec3 translation) {
  m_position += translation;
  recalculateView();
}

void Camera::rotate(float yaw, float pitch) {
  m_yaw += yaw;
  m_pitch -= pitch;
  m_pitch = glm::clamp(m_pitch, -glm::half_pi<float>() * 0.98f, glm::half_pi<float>() * 0.98f);

  recalculateDirections();
  recalculateView();
}

// Call this BEFORE calling recalculateView()
void Camera::recalculateDirections() {
  glm::vec3 forward{0.0f, 0.0f, -1.0f};
  glm::vec3 left{-1.0f, 0.0f, 0.0f};
  glm::vec3 up{0.0f, -1.0f, 0.0f};
  m_forward = glm::rotate(forward, m_pitch, left);
  m_forward = glm::rotate(m_forward, m_yaw, up);
  m_left = glm::normalize(glm::cross(m_forward, m_up));
}

void Camera::recalculateView() { m_view = glm::lookAt(m_position, m_position + m_forward, {0.0f, 1.0f, 0.0f}); }

void Camera::update(float dt) {
  float speed = m_speed;

  if (m_keyInput.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
    speed = m_speed * m_sprintFactor;
  } else {
    speed = m_speed;
  }

  glm::vec3 inputAxis{0.0f, 0.0f, 0.0f};

  if (m_keyInput.isKeyDown('W')) {
    inputAxis += m_forward;
  }
  if (m_keyInput.isKeyDown('S')) {
    inputAxis += -m_forward;
  }
  if (m_keyInput.isKeyDown('A')) {
    inputAxis += m_left;
  }
  if (m_keyInput.isKeyDown('D')) {
    inputAxis += -m_left;
  }
  if (m_keyInput.isKeyDown(GLFW_KEY_SPACE)) {
    inputAxis += m_up;
  }
  if (m_keyInput.isKeyDown(GLFW_KEY_LEFT_CONTROL)) {
    inputAxis += -m_up;
  }

  inputAxis = glm::clamp(inputAxis, {-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0});

  translate(inputAxis * speed * dt);

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