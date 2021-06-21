#pragma once

#include "ve_model.hpp"

#include <memory>

namespace ve {

struct Transform2DComponent {
  glm::vec2 translation{};
  glm::vec2 scale{1.0f, 1.0f};
  float rotation = 0.0f;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMat{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};

    return rotMat * scaleMat;
  }
};

class GameObject {
public:
  using id_t = unsigned int;

  static GameObject createGameObject() {
    static id_t currentId = 0;
    return GameObject{currentId++};
  }

  GameObject(const GameObject &) = delete;
  GameObject &operator=(const GameObject &) = delete;
  GameObject(GameObject &&) = default;
  GameObject &operator=(GameObject &&) = default;

  const id_t getID() { return m_id; }

  std::shared_ptr<Model> model{};
  glm::vec3 color{};
  Transform2DComponent transform2D{};

private:
  GameObject(id_t id) : m_id{id} {}
  id_t m_id;
};

} // namespace ve