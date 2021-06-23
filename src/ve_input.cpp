#include "ve_input.hpp"

#include <algorithm>
#include <cassert>

#include <iostream>

namespace ve {

// Key Input

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

// Mouse Input

GLFWwindow *MouseInput::m_window = nullptr;
std::vector<MouseInput *> MouseInput::m_instances{};
MouseInput::InputMode MouseInput::m_inputMode = MouseInput::InputMode::Absolute;

void MouseInput::setInputMode(InputMode mode) {
  assert(m_window != nullptr && "Can't set input mode until MouseInput::init() has been called.");
  assert(mode != InputMode::None && "Can't set input mode to InputMode::None");
  switch (mode) {
  case InputMode::Raw:
    assert(glfwRawMouseMotionSupported() && "This device does not support raw mouse motion");
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    MouseInput::m_inputMode = Raw;
    break;
  case InputMode::Absolute:
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    MouseInput::m_inputMode = Absolute;
    break;
  }
}

void MouseInput::init(GLFWwindow *window) {
  MouseInput::m_window = window;
  glfwSetMouseButtonCallback(window, MouseInput::mouseButtonCallback);
  glfwSetScrollCallback(window, MouseInput::mouseScrollCallback);
  glfwSetCursorPosCallback(window, MouseInput::cursorPositionCallback);
  MouseInput::setInputMode(Absolute);
}

MouseInput::MouseInput(std::vector<int> buttonsToMonitor, InputMode modeToWatch, bool position, bool scroll)
    : m_enabled{true}, m_modeToWatch{modeToWatch}, m_watchPosition{position},
      m_watchScroll{scroll}, m_xPos{0.0}, m_yPos{0.0}, m_xOffset{0.0}, m_yOffset{0.0} {
  for (int button : buttonsToMonitor) {
    m_buttons[button] = false;
  }

  m_instances.push_back(this);
}

MouseInput::MouseInput(std::vector<int> buttonsToMonitor)
    : m_enabled{true}, m_modeToWatch{MouseInput::InputMode::None}, m_watchScroll{false},
      m_watchPosition{false}, m_xPos{0.0}, m_yPos{0.0}, m_xOffset{0.0}, m_yOffset{0.0} {
  for (int button : buttonsToMonitor) {
    m_buttons[button] = false;
  }

  m_instances.push_back(this);
}

MouseInput::~MouseInput() {
  // remove this instance from the list of instances
  m_instances.erase(std::remove(m_instances.begin(), m_instances.end(), this), m_instances.end());
}

bool MouseInput::isButtonDown(int button) {
  bool result = false;
  if (m_enabled) {
    std::map<int, bool>::iterator it = m_buttons.find(button);
    if (it != m_buttons.end()) {
      result = m_buttons[button];
    }
  }
  return result;
}

void MouseInput::setIsButtonDown(int button, bool isDown) {
  std::map<int, bool>::iterator it = m_buttons.find(button);
  if (it != m_buttons.end()) {
    m_buttons[button] = isDown;
  }
}

void MouseInput::setMousePosition(double x, double y) {
  if (m_watchPosition && MouseInput::m_inputMode == m_modeToWatch) {
    m_xPos = x;
    m_yPos = y;
  } else {
    m_xPos = 0.0;
    m_yPos = 0.0;
  }
}
void MouseInput::setScrollOffset(double x, double y) {
  if (m_watchScroll) {
    m_xOffset = x;
    m_yOffset = y;
  } else {
    m_xOffset = 0.0;
    m_yOffset = 0.0;
  }
}

void MouseInput::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  for (MouseInput *mouseInput : m_instances) {
    mouseInput->setIsButtonDown(button, action != GLFW_RELEASE);
  }
}

void MouseInput::mouseScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
  for (MouseInput *mouseInput : m_instances) {
    mouseInput->setScrollOffset(xOffset, yOffset);
  }
}

void MouseInput::cursorPositionCallback(GLFWwindow *window, double x, double y) {
  for (MouseInput *mouseInput : m_instances) {
    mouseInput->setMousePosition(x, y);
  }
}

} // namespace ve