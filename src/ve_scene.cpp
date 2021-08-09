#include "ve_scene.hpp"

#include <algorithm>

namespace ve {

Scene::Scene(ModelLoader &modelLoader)
    : m_modelLoader{modelLoader} {}

void Scene::addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string modelPath) {
  GameObject object = GameObject::createGameObject();

  Model model = m_modelLoader.loadFromglTF(modelPath);
  object.model = model;

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
    if (lhs.model.indexCount < rhs.model.indexCount) {
      return true;
    }
    if (lhs.model.firstIndex < rhs.model.firstIndex) {
      return true;
    }
    return false;
  });

  uint32_t firstInstance = 0;
  uint32_t instanceCount = 0;
  size_t i = 0;
  Model currentModel = m_gameObjects[0].model;

  while (i < m_gameObjects.size()) {
    while (m_gameObjects[i].model == currentModel) {
      i++;
      instanceCount++;
    }
    DrawCall dc =
        {currentModel.indexCount, instanceCount, currentModel.firstIndex, currentModel.vertexOffset, firstInstance};
    m_drawCalls.push_back(dc);
    firstInstance = instanceCount;
    instanceCount = 0;
    currentModel = m_gameObjects[i].model;
  }
}

void Scene::draw(VkCommandBuffer cmd) {
  for (DrawCall &dc : m_drawCalls) {
    vkCmdDrawIndexed(cmd, dc.indexCount, dc.instanceCount, dc.firstIndex, dc.vertexOffset, dc.firstInstance);
  }
}

} // namespace ve