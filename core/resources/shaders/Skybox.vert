#version 450

layout (binding = 0) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    float renderID;
} objectUbo;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec3 outUVW;
layout (location = 1) out vec4 outNormal;

void main() 
{
    outUVW = inPosition.xyz;
    outNormal = vec4(0.0, 0.0, 0.0, objectUbo.renderID);
    gl_Position = objectUbo.projection * mat4(mat3(objectUbo.view * objectUbo.model)) * inPosition;
}