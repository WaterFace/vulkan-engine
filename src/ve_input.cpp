#include "ve_input.hpp"

#include <algorithm>

namespace ve {

std::vector<KeyInput *> KeyInput::m_instances{};

void KeyInput::init(GLFWwindow *window) { glfwSetKeyCallback(window, KeyInput::keyCallback); }

KeyInput::KeyInput(std::vector<int> keysToMonitor) : m_enabled{true} {
  for (int key : keysToMonitor) {
    m_keys[key] = false;
  }

  m_instances.push_back(this);
}

KeyInput::~KeyInput() {
  // remove this instance from the list of instances
  m_instances.erase(std::remove(m_instances.begin(), m_instances.end(), this), m_instances.end());
}

bool KeyInput::isKeyDown(int key) {
  bool result = false;
  if (m_enabled) {
    std::map<int, bool>::iterator it = m_keys.find(key);
    if (it != m_keys.end()) {
      result = m_keys[key];
    }
  }
  return result;
}

void KeyInput::setIsKeyDown(int key, bool isDown) {
  std::map<int, bool>::iterator it = m_keys.find(key);
  if (it != m_keys.end()) {
    m_keys[key] = isDown;
  }
}

void KeyInput::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  for (KeyInput *keyInput : m_instances) {
    keyInput->setIsKeyDown(key, action != GLFW_RELEASE);
  }
}

} // namespace ve