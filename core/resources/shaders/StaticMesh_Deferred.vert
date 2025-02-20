#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec4 cameraPos;
} objectUbo;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outBaseColor;
layout (location = 2) out vec3 outNormal;

void main() 
{
    outPosition = objectUbo.model * inPosition;
    outBaseColor = inColor;
    outNormal = inNormal;
    gl_Position = objectUbo.projection * objectUbo.view * outPosition;
}