#include "ve_scene.hpp"

#include <algorithm>

namespace ve {

Scene::Scene(MeshLoader &modelLoader)
    : m_modelLoader{modelLoader} {}

void Scene::addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string modelPath) {
  GameObject object = GameObject::createGameObject();

  Mesh model = m_modelLoader.loadFromglTF(modelPath);
  object.mesh = model;

  object.transform.translation = position;
  object.transform.rotation = rotation;
  object.transform.scale = scale;

  m_gameObjects.push_back(object);
}

void Scene::addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
  GameObject object = GameObject::createGameObject();

  object.transform.translation = position;
  object.transform.rotation = rotation;
  object.transform.scale = scale;

  m_gameObjects.push_back(object);
}

void Scene::addLight(PointLight light) { m_lights.push_back(light); }

void Scene::prepare() {
  if (m_gameObjects.size() == 0) {
    return;
  }

  m_drawCalls.clear();

  std::sort(m_gameObjects.begin(), m_gameObjects.end(), [](const GameObject &lhs, const GameObject &rhs) {
    if (lhs.mesh.primitiveCount < rhs.mesh.primitiveCount) {
      return true;
    }
    if (lhs.mesh.firstPrimitive < rhs.mesh.firstPrimitive) {
      return true;
    }
    return false;
  });

  uint32_t firstInstance = 0;
  uint32_t instanceCount = 0;
  size_t i = 0;
  Mesh currentMesh = m_gameObjects[0].mesh;

  while (i < m_gameObjects.size()) {
    while (m_gameObjects[i].mesh == currentMesh) {
      i++;
      instanceCount++;
    }
    DrawCall dc =
        {currentMesh.indexCount, instanceCount, currentMesh.firstIndex, currentMesh.vertexOffset, firstInstance};
    m_drawCalls.push_back(dc);
    firstInstance += instanceCount;
    instanceCount = 0;
    currentMesh = m_gameObjects[i].mesh;
  }
}

void Scene::draw(VkCommandBuffer cmd) {
  for (DrawCall &dc : m_drawCalls) {
    vkCmdDrawIndexed(cmd, dc.indexCount, dc.instanceCount, dc.firstIndex, dc.vertexOffset, dc.firstInstance);
  }
}

} // namespace ve