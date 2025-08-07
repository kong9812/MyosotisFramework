#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/RawDataLoader.glsl"
#include "Loader/MainCameraDataLoader.glsl"

layout (push_constant) uniform PushConstant {
    uint StandardSSBOIndex;
    uint TextureId;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outBaseColor;

layout (location = 3) out vec4 outRayDir;
layout (location = 4) out vec2 outUV;
layout (location = 5) out flat uint outRenderID;

void main() 
{
    MainCameraData cameraData = MainCameraDataLoader_GetMainCameraData();
    StandardSSBO standardSSBO = RawDataLoader_LoadStandardSSBO(StandardSSBOIndex);

    outPosition = standardSSBO.model * inPosition;
    outNormal = normalize(standardSSBO.model * vec4(inNormal, 0.0));
    outRenderID = standardSSBO.renderID;
    outUV = inUV;
    outBaseColor = inColor;
    outRayDir = outPosition - cameraData.position;
    gl_Position = cameraData.projection * cameraData.view * outPosition;
}