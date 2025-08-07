#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/RawDataLoader.glsl"
#include "Loader/MainCameraDataLoader.glsl"

layout(push_constant) uniform PushConstant {
    uint StandardSSBOIndex;
    uint TextureId;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec3 outUVW;
layout (location = 1) out flat uint outRenderID;

void main() 
{
    MainCameraData cameraData = MainCameraDataLoader_GetMainCameraData();
    StandardSSBO standardSSBO = RawDataLoader_LoadStandardSSBO(StandardSSBOIndex);

    outUVW = inPosition.xyz;
    outRenderID = standardSSBO.renderID;
    gl_Position = cameraData.projection * mat4(mat3(cameraData.view * standardSSBO.model)) * inPosition;
}