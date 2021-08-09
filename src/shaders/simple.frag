#version 450

// pos and normal coordinates are in world space
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragUV0;
layout(location = 4) in vec2 fragUV1;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Uniform{
  mat4 view;
  mat4 proj;
  mat4 viewproj;
  vec3 position;
} camera;

struct InstanceData {
  mat4 model;
  mat4 normalRotation;
};

layout(set = 0, binding = 1) buffer Instances{
  mat4 model;
} instanceData;

struct PointLight {
  vec3 position;

  vec3 diffuseColor;
  float diffusePower;

  vec3 specularColor;
  float specularPower;
};

layout(set = 0, binding = 2) buffer LightData{
  int numLights;
  PointLight lights[];
} lights;

void main() {
  // these would be specified by the material
  const vec3 k_s = vec3(1.0f, 1.0f, 1.0f);
  const vec3 k_d = vec3(0.3f, 0.3f, 0.3f);
  const vec3 k_a = vec3(0.05f, 0.05f, 0.05f);
  const float shininess = 30;

  vec3 color = k_a * fragColor;

  vec3 V = normalize(camera.position - fragPosition);
  for (int i = 0; i < lights.numLights; i++) {
    vec3 lightPos = lights.lights[i].position;
    vec3 lightDir = lightPos - fragPosition;
    float dist = length(lightDir);
    dist *= dist;
    lightDir = normalize(lightDir);

    vec3 normal = normalize(fragNormal);

    float lambertian = max(dot(lightDir, normal), 0.0f);
    float specular = 0.0f;

    if (lambertian > 0.0) {
      vec3 viewDir = normalize(camera.position - fragPosition);

      vec3 halfDir = normalize(lightDir + viewDir);
      float specAngle = max(dot(halfDir, normal), 0.0f);
      specular = pow(specAngle, shininess);
    }

    vec3 diffuseColor = lights.lights[i].diffuseColor;
    float diffusePower = lights.lights[i].diffusePower;
    color += k_d * lambertian * diffuseColor * diffusePower / dist;

    vec3 specularColor = lights.lights[i].specularColor;
    float specularPower = lights.lights[i].specularPower;
    color += k_s * specular * specularColor * specularPower / dist;
  }

  outColor = vec4(color, 1.0);
}