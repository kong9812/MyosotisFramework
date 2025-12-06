#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outWorldPos;
layout (location = 1) out vec3 outWorldNormal;


void main() 
{
    // CameraInfo (MainCameraData) を取得
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    outWorldPos = cameraData.projection * cameraData.view * inPosition;
    outWorldNormal = inNormal;
    gl_Position = outWorldPos; 
}