#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "RawDataLoader.glsl"

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
layout (location = 2) out vec4 outBaseColor;

layout (location = 3) out vec4 outRayDir;
layout (location = 4) out vec2 outUV;
layout (location = 5) out flat uint outRenderID;

void main() 
{
    BaseObjectData meta = objectTable[nonuniformEXT(objectIndex)];

    mat4 model = LoadMat4(meta.dataOffset + 0);
    mat4 view = LoadMat4(meta.dataOffset + 16);
    mat4 projection = LoadMat4(meta.dataOffset + 32);
    vec4 color = LoadVec4(meta.dataOffset + 48);
    uint renderID = rawData[meta.dataOffset + 52];
    vec4 cameraPosition = LoadVec4(meta.dataOffset + 53);

    outPosition = model * inPosition;
    outNormal = normalize(model * vec4(inNormal, 0.0));
    outRenderID = renderID;
    outUV = inUV;
    outBaseColor = inColor;
    outRayDir = outPosition - cameraPosition;
    gl_Position = projection * view * outPosition;
}