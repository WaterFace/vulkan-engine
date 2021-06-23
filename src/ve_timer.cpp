#include "ve_timer.hpp"

#include <algorithm>

namespace ve {

constexpr float Timer::MAX_DELTA_TIME;

Timer::Timer() {
  m_start = std::chrono::high_resolution_clock::now();
  m_previous = m_start;
}

float Timer::dt() {
  float ft = std::chrono::duration<float, std::chrono::seconds::period>(m_current - m_previous).count();
  return std::min(ft, Timer::MAX_DELTA_TIME);
}
float Timer::elapsed() {
  return std::chrono::duration<float, std::chrono::seconds::period>(m_current - m_start).count();
}
float Timer::frameTime() {
  return std::chrono::duration<float, std::chrono::seconds::period>(m_current - m_previous).count();
}

void Timer::update() {
  m_previous = m_current;
  m_current = std::chrono::high_resolution_clock::now();
}
} // namespace ve