#version 450

layout (binding = 1) uniform samplerCube colorMap;

layout (location = 0) in vec3 inUVW;
layout (location = 1) in flat uint inRenderID;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outBaseColor;
layout (location = 3) out uint outRenderID;

void main() 
{
    outRenderID = inRenderID;
    outBaseColor = texture(colorMap, inUVW);
}