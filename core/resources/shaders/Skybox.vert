#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec3 outUVW;

void main() 
{
    // CameraInfo (MainCameraData)
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());

    mat4 view = cameraData.view;
    view[3] = vec4(0, 0, 0, 1); 

    vec4 pos = cameraData.projection * view * inPosition;    

    // skybox を奥に固定
    gl_Position = pos.xyww; 

    outUVW = inPosition.xyz;
}