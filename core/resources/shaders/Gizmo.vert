#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout(push_constant) uniform PushConstant {
    mat4 model;
    vec4 color;
};

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
    // CameraInfo (MainCameraData)
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());

    gl_Position = cameraData.projection * cameraData.view * model * vec4(inPosition, 1.0);

    outColor = color;
}