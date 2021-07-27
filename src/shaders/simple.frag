#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

struct InstanceData {
  mat4 model;
};

layout(set = 0, binding = 0) uniform Uniform{
  mat4 view;
  mat4 proj;
  mat4 viewproj;
} camera;

layout(set = 0, binding = 1) buffer Instances{
  mat4 model;
} instanceData;

void main() {
    outColor = vec4(fragColor, 1.0);
}