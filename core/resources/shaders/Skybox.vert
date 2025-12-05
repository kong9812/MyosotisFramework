#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

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
    debugPrintfEXT("Hello from shader!\ninPosition: %f %f %f %f", inPosition.x, inPosition.y, inPosition.z, inPosition.w);

    mat4 view = cameraData.view;
    view[3] = vec4(0, 0, 0, 1); 

    vec4 pos = cameraData.projection * view * inPosition;    

    // skybox を奥に固定
    gl_Position = pos.xyww; 

    outUVW = inPosition.xyz;
}