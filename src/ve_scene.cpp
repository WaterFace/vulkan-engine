#include "ve_scene.hpp"

namespace ve {

Scene::Scene(ModelLoader &modelLoader) : m_modelLoader{modelLoader} {}

void Scene::addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string modelPath) {
  GameObject object = GameObject::createGameObject();

  if (m_loadedModels.find(modelPath) == m_loadedModels.end()) {
    Model model = m_modelLoader.loadFromglTF(modelPath);
    m_loadedModels[modelPath] = model;
    object.model = model;
  } else {
    object.model = m_loadedModels[modelPath];
  }

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

void Scene::prepare() {}

void Scene::draw() {}

} // namespace ve