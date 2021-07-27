#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

struct InstanceData {
  mat4 model;
};

layout(set = 0, binding = 0) uniform Uniform{
  mat4 view;
  mat4 proj;
  mat4 viewproj;
} camera;

layout(set = 0, binding = 1) buffer Instances{
  InstanceData instances[];
} data;

void main() {
  gl_Position = camera.viewproj * data.instances[gl_InstanceIndex].model * vec4(position, 1.0f);
  fragColor = color;
}