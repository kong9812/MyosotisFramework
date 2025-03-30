#version 450

layout (binding = 1) uniform sampler2D normalMap;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inBaseColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outBaseColor;

void main() 
{
    outPosition = inPosition;
    outNormal = inNormal;
    outBaseColor = inBaseColor;
}