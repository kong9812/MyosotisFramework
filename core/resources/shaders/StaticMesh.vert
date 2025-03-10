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

void main() {
    outColor = inColor;
    gl_Position = ubo.projection * ubo.view * ubo.model * inPosition;
}