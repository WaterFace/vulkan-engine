#pragma once

#include "ve_game_object.hpp"
#include "ve_light.hpp"
#include "ve_model.hpp"
#include "ve_model_loader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

namespace ve {

class Scene {
public:
  Scene(ModelLoader &modelLoader);
  ~Scene(){};

  void addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string modelPath);
  void addGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

  void addLight(PointLight light);

  // for now this just sorts the list of models
  void prepare();
  void draw();

private:
  ModelLoader &m_modelLoader;
  std::unordered_map<std::string, Model> m_loadedModels;
  std::vector<PointLight> m_lights;
  std::vector<GameObject> m_gameObjects;
};

} // namespace ve