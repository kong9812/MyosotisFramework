#version 450
#extension GL_GOOGLE_include_directive : require
#include "SSBO/DirectionalLightSSBO.glsl"

layout(push_constant) uniform PushConstant {
    uint RawDataMetaDataOffset;
    uint StandardSSBOIndex;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    RawDataMetaData meta = RawDataLoader_GetRawDataMetaData(RawDataMetaDataOffset);
    DirectionalLightSSBO directionalLightSSBO = DirectionalLightSSBO_LoadDirectionalLightSSBO(meta.dataOffset + 0);
    StandardSSBO standardSSBO = RawDataLoader_LoadStandardSSBO(StandardSSBOIndex);

    gl_Position = directionalLightSSBO.viewProjection * standardSSBO.model * inPosition;
}