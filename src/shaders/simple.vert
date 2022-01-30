#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv0;
layout(location = 4) in vec2 uv1;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV0;
layout(location = 4) out vec2 fragUV1;
layout(location = 5) flat out int primitiveIndex;

layout(set = 0, binding = 0) uniform Uniform{
  mat4 view;
  mat4 proj;
  mat4 viewproj;
  vec3 position;
} camera;

struct Object {
  mat4 model;
  mat4 normalRotation;
};

layout(set = 0, binding = 1) buffer Objects{
  Object object[];
} objectData;

struct Primitive {
  uint parentObject;
  int material;
};

layout (set = 0, binding = 2) buffer Primitives{
  Primitive primitive[];
} primitiveData;

struct PointLight {
  vec3 position;

  vec3 diffuseColor;
  float diffusePower;

  vec3 specularColor;
  float specularPower;
};

layout(set = 0, binding = 3) buffer Lights{
  int numLights;
  PointLight light[];
} lightData;

void main() {
  Primitive primitive = primitiveData.primitive[gl_InstanceIndex];
  Object parentObject = objectData.object[primitive.parentObject];
  gl_Position = camera.viewproj * parentObject.model *
                vec4(position, 1.0f);

  fragPosition =
      (parentObject.model * vec4(position, 1.0f)).xyz;
  fragColor = color;
  fragNormal =
      (parentObject.normalRotation * vec4(normal, 0.0f))
          .xyz;
  fragUV0 = uv0;
  fragUV1 = uv1;
  primitiveIndex = gl_InstanceIndex;
}