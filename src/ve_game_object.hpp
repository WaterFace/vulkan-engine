#pragma once

#include "ve_mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace ve {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 rotation{};

  const glm::mat4 mat4() const {
    glm::mat4 m{1.0f};

    m = glm::translate(m, translation);

    m = glm::rotate(m, rotation.y, {0.0f, 1.0f, 0.0f});
    m = glm::rotate(m, rotation.x, {1.0f, 0.0f, 0.0f});
    m = glm::rotate(m, rotation.z, {0.0f, 0.0f, 1.0f});

    m = glm::scale(m, scale);
    return m;
  }
};

class GameObject {
public:
  using id_t = unsigned int;

  static GameObject createGameObject() {
    static id_t currentId = 0;
    return GameObject{currentId++};
  }

  const id_t getID() { return m_id; }

  Mesh mesh{};
  glm::vec3 color{};
  TransformComponent transform{};

private:
  GameObject(id_t id)
      : m_id{id} {}
  id_t m_id;
};

} // namespace ve