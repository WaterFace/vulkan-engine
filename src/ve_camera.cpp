#include "ve_camera.hpp"

#include <cassert>
#include <limits>

namespace ve {

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
  m_projection = glm::ortho(left, right, bottom, top);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  m_projection = glm::perspective(fovy, aspect, near, far);
}

const glm::mat4 Camera::getView() const { return transform.mat4(); }

void Camera::update() {
  if (keyInput.isKeyDown('W')) {
    transform.translation.z += m_speed;
  }
  if (keyInput.isKeyDown('S')) {
    transform.translation.z -= m_speed;
  }
  if (keyInput.isKeyDown('A')) {
    transform.translation.x += m_speed;
  }
  if (keyInput.isKeyDown('D')) {
    transform.translation.x -= m_speed;
  }
}

} // namespace ve