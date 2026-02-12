#version 330 core
out vec4 FragColor;
in vec3 bNormal;
in vec3 bPos;

uniform vec3 cameraPos;

struct DirectLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
};

struct FlashLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float angle;
    float cutoff;
};

struct Material {
    vec3 color;
};

#define DIRECT_LIGHT_COUNT 8
uniform DirectLight directLights[DIRECT_LIGHT_COUNT];
#define POINT_LIGHT_COUNT 8
uniform PointLight pointLights[POINT_LIGHT_COUNT];
#define FLASH_LIGHT_COUNT 2
uniform FlashLight flashLights[FLASH_LIGHT_COUNT];

uniform Material material;

vec3 DirectLightCalculate(int i, vec3 normal) {
    float factor = dot(normal, -directLights[i].direction);
    return max(vec3(0.0), factor*directLights[i].color);
}

vec3 PointLightCalculate(int i, vec3 position, vec3 normal) {
    vec3 to = pointLights[i].position - position;
    vec3 direction = normalize(to);
    float dist = length(to);
    float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist);

    float factor = dot(normal, direction)*attenuation;
    return max(vec3(0.0), factor*pointLights[i].color);
}

vec3 FlashLightCalculate(int i, vec3 position, vec3 normal) {
    vec3 to = flashLights[i].position - position;

    vec3 direction = normalize(to);
    float theta = dot(direction, -normalize(flashLights[i].direction));
    float inner_edge = cos(flashLights[i].angle);
    float outer_edge = cos(flashLights[i].angle + flashLights[i].cutoff);
    float epsilon = inner_edge - outer_edge;
    float intensity = clamp((theta - outer_edge)/epsilon, 0.0, 1.0);
    
    float dist = length(to);
    float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.1 * dist * dist);
    
    float factor = dot(normal, direction)*attenuation*intensity;
    return max(vec3(0.0), factor*flashLights[i].color);
}

void main()
{
    vec3 position = bPos;
    vec3 normal = normalize(bNormal);
    vec3 result = vec3(0.0);
    for (int i = 0; i < DIRECT_LIGHT_COUNT; i++) {
	result += DirectLightCalculate(i, normal);
    }
    for (int i = 0; i < POINT_LIGHT_COUNT; i++) {
	result += PointLightCalculate(i, position, normal);
    }
    for (int i = 0; i < FLASH_LIGHT_COUNT; i++) {
	result += FlashLightCalculate(i, position, normal);
    }
    
    FragColor = vec4(result, 1.0);
    FragColor = pow(FragColor, vec4(vec3(1./2.2), 1));
}
