#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace ve {

class Window {
public:
  Window(int w, int h, std::string name);
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(m_window); }
  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
  }

  bool wasWindowResized() { return m_framebufferResized; }
  void resetWindowResizedFlag() { m_framebufferResized = false; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  static uint32_t m_window_count;
  static void framebufferResizedCallback(GLFWwindow *window, int width,
                                         int height);

  void initWindow();

  int m_width;
  int m_height;
  bool m_framebufferResized = false;

  std::string m_name;
  GLFWwindow *m_window;
};

} // namespace ve