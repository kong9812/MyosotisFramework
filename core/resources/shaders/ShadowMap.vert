#version 450
#extension GL_GOOGLE_include_directive : require

#include "SSBO/StandardSSBO.glsl"
#include "SSBO/DirectionalLightSSBO.glsl"

layout(push_constant) uniform PushConstant {
    uint objectIndex;
    uint textureId;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    RawDataMetaData meta = RawDataLoader_GetRawDataMetaData(objectIndex);
    StandardSSBO standardSSBO = StandardSSBO_LoadStandardSSBO(meta.dataOffset + 0);
    DirectionalLightSSBO directionalLightSSBO = DirectionalLightSSBO_LoadDirectionalLightSSBO(meta.dataOffset + StandardSSBOSize);

    gl_Position = directionalLightSSBO.viewProjection * standardSSBO.model * inPosition;
}