#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec4 cameraPos;
} ubo;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec4 worldPosition = ubo.model * inPosition;
    gl_Position = ubo.projection * ubo.view * worldPosition;
    vec4 color = inColor;
    float dist = distance(ubo.cameraPos.xyz, worldPosition.xyz);
    color.a = 1.0 - smoothstep(20.0, 30.0, dist);
    outColor = color;
}