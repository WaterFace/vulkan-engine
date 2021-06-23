#pragma once

#include <GLFW/glfw3.h>

#include <map>
#include <vector>

namespace ve {

class KeyInput {
public:
  KeyInput(std::vector<int> keysToMonitor);
  ~KeyInput();

  bool isKeyDown(int key);
  bool isEnabled() { return m_enabled; };
  void setEnabled(bool enabled) { m_enabled = enabled; }

  static void init(GLFWwindow *window);

private:
  bool m_enabled;
  void setIsKeyDown(int key, bool isDown);
  std::map<int, bool> m_keys;

  static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static std::vector<KeyInput *> m_instances;
};

class MouseInput {
public:
  enum InputMode { None, Raw, Absolute };

  MouseInput(std::vector<int> buttonsToMonitor, bool position, bool scroll);
  MouseInput(std::vector<int> buttonsToMonitor);
  ~MouseInput();

  bool isButtonDown(int button);
  double x() { return m_xPos; }
  double y() { return m_yPos; }

  double xScroll() { return m_xOffset; }
  double yScroll() { return m_yOffset; }
  bool isEnabled() { return m_enabled; };
  void setEnabled(bool enabled) { m_enabled = enabled; }

  static InputMode getInputMode() { return m_inputMode; };
  static void setInputMode(InputMode mode);
  static void init(GLFWwindow *window);

private:
  bool m_enabled;
  bool m_watchPosition;
  bool m_watchScroll;
  void setIsButtonDown(int button, bool isDown);
  void setMousePosition(double x, double y);
  void setScrollOffset(double x, double y);
  std::map<int, bool> m_buttons;
  double m_xPos, m_yPos;
  double m_xOffset, m_yOffset;

  static void cursorPositionCallback(GLFWwindow *window, double x, double y);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
  static void mouseScrollCallback(GLFWwindow *window, double xOffset, double yOffset);

  static GLFWwindow *m_window;
  static InputMode m_inputMode;
  static std::vector<MouseInput *> m_instances;
};

} // namespace ve