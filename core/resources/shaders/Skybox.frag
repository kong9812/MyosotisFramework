#version 450

layout (binding = 1) uniform samplerCube colorMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outBaseColor;

void main() 
{
    outPosition = vec4(0.0);
    outColor = texture(colorMap, inUVW);
}