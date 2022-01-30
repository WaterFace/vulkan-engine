#pragma once

#include "ve_game_object.hpp"
#include "ve_light.hpp"
#include "ve_mesh.hpp"
#include "ve_mesh_loader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

namespace ve {

struct DrawCall {
  uint32_t indexCount;
  uint32_t instanceCount;
  uint32_t firstIndex;
  int32_t vertexOffset;
  uint32_t firstInstance;
};

class Scene {
public:
  Scene(MeshLoader &modelLoader);
  ~Scene(){};

  void addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string modelPath);
  void addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

  void addLight(PointLight light);

  const std::vector<GameObject> &gameObjects() { return m_gameObjects; }
  const std::vector<PointLight> &lights() { return m_lights; }

  // this sorts the list of `GameObject`s by mesh, then
  // fills the `m_drawCalls` vector with the data to make
  // draw calls in `draw()`
  void prepare();
  void draw(VkCommandBuffer cmd);

private:
  MeshLoader &m_modelLoader;

  std::vector<DrawCall> m_drawCalls;
  std::vector<PointLight> m_lights;
  std::vector<GameObject> m_gameObjects;
};

} // namespace ve