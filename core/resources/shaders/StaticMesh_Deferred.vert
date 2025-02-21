#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform StandardUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec4 cameraPos;
} objectUbo;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outBaseColor;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outCameraPosition;

void main() 
{
    outPosition = objectUbo.model * inPosition;
    outBaseColor = inColor;
    outNormal = normalize(objectUbo.model * vec4(inNormal, 0.0));
    outCameraPosition = objectUbo.cameraPos;
    gl_Position = objectUbo.projection * objectUbo.view * outPosition;
}