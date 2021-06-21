#include "ve_camera.hpp"

#include <cassert>
#include <limits>

namespace ve {

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
  // m_projection = glm::mat4 { 1.0f };
  // m_projection[0][0] = 2.f / (right - left);
  // m_projection[1][1] = 2.f / (bottom - top);
  // m_projection[2][2] = 1.f / (far - near);
  // m_projection[3][0] = -(right + left) / (right - left);
  // m_projection[3][1] = -(bottom + top) / (bottom - top);
  // m_projection[3][2] = -near / (far - near);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
  // assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  // const float tanHalfFovy = tan(fovy / 2.f);
  // m_projection = glm::mat4 { 0.0f };
  // m_projection[0][0] = 1.f / (aspect * tanHalfFovy);
  // m_projection[1][1] = 1.f / (tanHalfFovy);
  // m_projection[2][2] = far / (far - near);
  // m_projection[2][3] = 1.f;
  // m_projection[3][2] = -(far * near) / (far - near);

  m_projection = glm::perspective(fovy, aspect, near, far);
}

}