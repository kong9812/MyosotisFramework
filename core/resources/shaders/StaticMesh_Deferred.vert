#version 450
#extension GL_EXT_nonuniform_qualifier : require

// Meta情報構造体
struct BaseObjectData {
    uint typeID;        // 今は使わない
    uint dataOffset;    // 実データのoffset
};

// Meta情報テーブル
layout(std430, binding = 0) buffer MetaBuffer {
    BaseObjectData objectTable[];
};

// 実データ
layout(std430, binding = 1) buffer AllDataBuffer {
    uint rawData[]; // 全てのデータをここにまとめる
};

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

mat4 LoadMat4(uint base) {
    return mat4(
        vec4(uintBitsToFloat(rawData[base +  0]), uintBitsToFloat(rawData[base +  1]),
             uintBitsToFloat(rawData[base +  2]), uintBitsToFloat(rawData[base +  3])),
        vec4(uintBitsToFloat(rawData[base +  4]), uintBitsToFloat(rawData[base +  5]),
             uintBitsToFloat(rawData[base +  6]), uintBitsToFloat(rawData[base +  7])),
        vec4(uintBitsToFloat(rawData[base +  8]), uintBitsToFloat(rawData[base +  9]),
             uintBitsToFloat(rawData[base + 10]), uintBitsToFloat(rawData[base + 11])),
        vec4(uintBitsToFloat(rawData[base + 12]), uintBitsToFloat(rawData[base + 13]),
             uintBitsToFloat(rawData[base + 14]), uintBitsToFloat(rawData[base + 15]))
    );
}
vec4 LoadVec4(uint base) {
    return vec4(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1]),
        uintBitsToFloat(rawData[base + 2]),
        uintBitsToFloat(rawData[base + 3])
    );
}

void main() 
{
    BaseObjectData meta = objectTable[nonuniformEXT(objectIndex)];

    mat4 model = LoadMat4(meta.dataOffset + 0);
    mat4 view = LoadMat4(meta.dataOffset + 16);
    mat4 projection = LoadMat4(meta.dataOffset + 32);
    vec4 color = LoadVec4(meta.dataOffset + 48);
    uint renderID = uint(rawData[meta.dataOffset + 52]); // floatをuintに変換

    outPosition = model * inPosition;
    outNormal = normalize(model * vec4(inNormal, 0.0));
    outRenderID = renderID;
    outUV = inUV;
    outBaseColor = inColor;
    gl_Position = projection * view * outPosition;
}