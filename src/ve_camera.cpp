#include "ve_camera.hpp"

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

const glm::mat4 Camera::getView() const { return transform.mat4(); }

void Camera::update(float dt) {
  if (m_keyInput.isKeyDown('W')) {
    transform.translation.z += m_speed * dt;
  }
  if (m_keyInput.isKeyDown('S')) {
    transform.translation.z -= m_speed * dt;
  }
  if (m_keyInput.isKeyDown('A')) {
    transform.translation.x += m_speed * dt;
  }
  if (m_keyInput.isKeyDown('D')) {
    transform.translation.x -= m_speed * dt;
  }

  if (m_keyInput.isKeyDown(GLFW_KEY_ESCAPE)) {
    if (MouseInput::getInputMode() == MouseInput::InputMode::Raw) {
      MouseInput::setInputMode(MouseInput::InputMode::Absolute);
    } else if (MouseInput::getInputMode() == MouseInput::InputMode::Absolute) {
      MouseInput::setInputMode(MouseInput::InputMode::Raw);
    }
  }

  static double prevX = m_mouseInput.x();
  static double prevY = m_mouseInput.y();
  double x = m_mouseInput.x() - prevX;
  double y = m_mouseInput.y() - prevY;
  prevX = m_mouseInput.x();
  prevY = m_mouseInput.y();

  transform.rotation.y += x * 0.01f;
  transform.rotation.x += y * -0.01f;
}

} // namespace ve