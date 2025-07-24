#version 450
#extension GL_GOOGLE_include_directive : require

#include "StandardSSBO.glsl"

layout(push_constant) uniform PushConstant {
    uint objectIndex;
    uint textureId;
};

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outBaseColor;
layout (location = 4) out flat uint outRenderID;

void main() 
{
    BaseObjectData meta = GetBaseObjectData(objectIndex);
    StandardSSBO standardSSBO = LoadStandardSSBO(meta.dataOffset + 0);

    outPosition = standardSSBO.model * inPosition;
    outNormal = normalize(standardSSBO.model * vec4(inNormal, 0.0));
    outRenderID = standardSSBO.renderID;
    outUV = inUV;
    outBaseColor = inColor;
    gl_Position = standardSSBO.projection * standardSSBO.view * outPosition;
}