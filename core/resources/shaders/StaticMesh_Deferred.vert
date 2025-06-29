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

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outBaseColor;

void main() 
{
    outPosition = objectUbo.model * inPosition;
    outNormal = normalize(objectUbo.model * vec4(inNormal, 0.0));
    outNormal.w = objectUbo.renderID;
    outUV = inUV;
    outBaseColor = inColor;
    gl_Position = objectUbo.projection * objectUbo.view * outPosition;
}