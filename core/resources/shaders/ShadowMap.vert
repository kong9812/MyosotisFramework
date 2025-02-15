#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(binding = 0) uniform LightUBO {
    mat4 viewProjection;
} lightUbo;

layout(binding = 1) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec4 cameraPos;
} objectUbo;

layout(location = 0) out vec4 outColor;

void main() 
{
    gl_Position = lightUbo.viewProjection * objectUbo.model * inPosition;
}