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

} // namespace ve