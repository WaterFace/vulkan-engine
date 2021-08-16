#version 450

#define MATH_PI 3.1415926535897932384626433832795

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

layout(set = 1, binding = 0) uniform sampler samp;
layout(set = 1, binding = 1) uniform texture2D textures[5];

vec3 getNormal() {
  vec3 tangentNormal = texture(sampler2D(textures[3], samp), fragUV0).xyz;

  vec3 q1 = dFdx(fragPosition);
	vec3 q2 = dFdy(fragPosition);
	vec2 st1 = dFdx(fragUV0);
	vec2 st2 = dFdy(fragUV0);

	vec3 N = normalize(fragNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

// implementation from https://learnopengl.com/PBR/Lighting

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1-F0) * pow(max(1-cosTheta, 0), 5);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0);
  float NdotH2 = NdotH*NdotH;

  float num = a2;
  float denom = (NdotH2 * (a2-1) + 1);
  denom = MATH_PI * denom * denom;

  return num/denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness+1);
  float k = (r*r) / 8.0;

  float num = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return num/denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N,V), 0);
  float NdotL = max(dot(N,L), 0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

void main() {
  vec3 N = getNormal();
  vec3 V = normalize(camera.position - fragPosition);

  vec3 albedo = texture(sampler2D(textures[0], samp), fragUV0).rgb;
  albedo = pow(albedo, vec3(2.2));
  float metallic = texture(sampler2D(textures[2], samp), fragUV0).r;
  float roughness = texture(sampler2D(textures[4], samp), fragUV0).r;

  vec3 Lo = vec3(0.0);
  for(int i = 0; i < lights.numLights; i++) {
    PointLight light = lights.lights[i];
    vec3 L = normalize(light.position - fragPosition);
    vec3 H = normalize(V+L);

    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (distance*distance);
    vec3 radiance = light.diffuseColor * attenuation;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0) * max(dot(N, L), 0);
    vec3 specular = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1) - kS;

    kD *= 1 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / MATH_PI + specular) * radiance * NdotL;
  }

  vec3 ambient = vec3(0.03) * albedo;
  vec3 color = ambient + Lo;

  color = color/(color+vec3(1));
  color = pow(color, vec3(1/2.2));

  outColor = vec4(color, 1.0);
}