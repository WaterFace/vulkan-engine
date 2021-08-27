#pragma once

#include "ve_device.hpp"
#include "ve_mesh.hpp"
#include "ve_mesh_loader.hpp"
#include "ve_texture.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tinygltf/tiny_gltf.h"

namespace ve {
namespace glTF {
struct Node;
struct BoundingBox {};
struct TextureSampler {
  VkFilter magFilter;
  VkFilter minFilter;
  VkSamplerAddressMode addressModeU;
  VkSamplerAddressMode addressModeV;
  VkSamplerAddressMode addressModeW;
  static TextureSampler defaultSampler() {
    return {
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT};
  }
};
struct Texture {
  bool isExternalTexture;
  std::string texturePath;
  std::vector<unsigned char> rawData;
  int width;
  int height;
};
struct Material {
  float metallicFactor = 1.0f;
  float roughnessFactor = 1.0f;
  glm::vec4 baseColorFactor = glm::vec4(1.0f);
  glm::vec4 emissiveFactor = glm::vec4(1.0f);

  uint32_t baseColorTexture;
  uint32_t metallicRoughnessTexture;
  uint32_t normalTexture;
  uint32_t occlusionTexture;
  uint32_t emissiveTexture;
};
struct Primitive {
  uint32_t firstIndex;
  uint32_t indexCount;
  uint32_t vertexCount;
  int32_t material;
  bool hasIndices;
  Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, int32_t material);
};
struct Mesh {
  std::vector<Primitive *> primitives;
  Mesh(glm::mat4 matrix){};
  ~Mesh();
};
struct Skin {};
struct Node {
  Node *parent;
  uint32_t index;
  std::vector<Node *> children;
  glm::mat4 matrix;
  std::string name;
  Mesh *mesh;
  Skin *skin;
  int32_t skinIndex{-1};
  glm::vec3 translation{};
  glm::vec3 scale{1.0f};
  glm::quat rotation{};
  glm::mat4 localMatrix();
  glm::mat4 getMatrix();
  void update();
  ~Node();
};
struct AnimationChannel {};
struct AnimationSampler {};
struct Animation {};
struct Model {
  glm::mat4 aabb;

  std::vector<Node *> nodes;
  std::vector<Node *> linearNodes;

  std::vector<Skin *> skins;
  std::vector<Texture> textures;
  std::vector<TextureSampler> textureSamplers;
  std::vector<Material> materials;
  std::vector<Animation> animations;
  std::vector<std::string> extensions;

  std::vector<ve::Mesh::IndexType> indexBuffer;
  std::vector<ve::Mesh::Vertex> vertexBuffer;

  struct Dimensions {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
  } dimensions;

  void loadNode(
      glTF::Node *parent,
      const tinygltf::Node &node,
      uint32_t nodeIndex,
      const tinygltf::Model &model,
      std::vector<ve::Mesh::IndexType> &indexBuffer,
      std::vector<ve::Mesh::Vertex> &vertexBuffer,
      float globalscale);
  void loadSkins(tinygltf::Model &gltfModel);
  void loadTextures(tinygltf::Model &gltfModel);
  VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
  VkFilter getVkFilterMode(int32_t filterMode);
  void loadTextureSamplers(tinygltf::Model &gltfModel);
  void loadMaterials(tinygltf::Model &gltfModel);
  void loadAnimations(tinygltf::Model &gltfModel);
  void loadFromFile(const std::string &filename, float scale = 1.0f);
  void calculateBoundingBox(Node *node, Node *parent);
  void getSceneDimensions();
  void updateAnimation(uint32_t index, float time);
  Node *findNode(Node *parent, uint32_t index);
  Node *nodeFromIndex(uint32_t index);
};

} // namespace glTF
} // namespace ve