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

layout(location = 0) out vec4 outColor;

void main() 
{
    BaseObjectData meta = objectTable[nonuniformEXT(objectIndex)];

    mat4 model = LoadMat4(meta.dataOffset + 0);
    mat4 view = LoadMat4(meta.dataOffset + 16);
    mat4 projection = LoadMat4(meta.dataOffset + 32);
    vec4 color = LoadVec4(meta.dataOffset + 48);
    uint renderID = uint(rawData[meta.dataOffset + 52]); // floatをuintに変換

    mat4 viewProjection = LoadMat4(meta.dataOffset + 53);
    vec4 position = LoadVec4(meta.dataOffset + 69);
    uint pcfCount = uint(rawData[meta.dataOffset + 73]);

    gl_Position = viewProjection * model * inPosition;
}