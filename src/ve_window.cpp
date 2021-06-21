#include "ve_window.hpp"

#include <stdexcept>

namespace ve {

uint32_t Window::m_window_count = 0;

Window::Window(int w, int h, std::string name)
    : m_width { w }
    , m_height { h }
    , m_name { name }
{
  initWindow();
  if (m_window) {
    m_window_count++;
  }
}

void Window::initWindow()
{
  if (m_window_count == 0) {
    glfwInit();
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, framebufferResizedCallback);
}

Window::~Window()
{
  glfwDestroyWindow(m_window);
  m_window_count--;
  if (m_window_count == 0) {
    glfwTerminate();
  }
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
  if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface");
  }
}

void Window::framebufferResizedCallback(GLFWwindow* window, int width, int height)
{
  auto veWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  veWindow->m_framebufferResized = true;
  veWindow->m_width = width;
  veWindow->m_height = height;
}

} // namespace ve