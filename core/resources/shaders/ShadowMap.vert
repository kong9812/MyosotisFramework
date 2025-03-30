#version 450

layout(binding = 0) uniform LightUBO {
    mat4 viewProjection;
    vec4 position;
    uint pcfCount;
} lightUbo;
layout(binding = 1) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
} objectUbo;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    gl_Position = lightUbo.viewProjection * objectUbo.model * inPosition;
}