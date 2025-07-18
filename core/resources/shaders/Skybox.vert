#version 450

layout (binding = 0) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    uint renderID;
} objectUbo;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec3 outUVW;
layout (location = 1) out flat uint outRenderID;

void main() 
{
    outUVW = inPosition.xyz;
    outRenderID = objectUbo.renderID;
    gl_Position = objectUbo.projection * mat4(mat3(objectUbo.view * objectUbo.model)) * inPosition;
}