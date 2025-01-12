#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = ubo.color;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
}