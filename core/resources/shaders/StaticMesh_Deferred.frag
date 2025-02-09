#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inBaseColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outBaseColor;

void main() 
{
    outPosition = inPosition;
    outBaseColor = inBaseColor;
    outColor = vec4(0.0);
}