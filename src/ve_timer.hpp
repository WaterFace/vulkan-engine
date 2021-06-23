#pragma once

#include <chrono>

namespace ve {

class Timer {
public:
  using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

  Timer();
  ~Timer(){};

  float dt();
  float elapsed();
  float frameTime();

  void update();

  static constexpr float MAX_DELTA_TIME = 1.0f / 10.0f;

private:
  time_t m_start;
  time_t m_current;
  time_t m_previous;
};

} // namespace ve