#ifndef RAWDATALOADER
#define RAWDATALOADER
#extension GL_EXT_nonuniform_qualifier : require

struct RawDataMetaData {
    uint typeID;
    uint dataOffset;
};

struct StandardSSBO {
    mat4 model;
    vec4 position;
    vec4 rotation;
    vec4 scale;
    vec4 color;
    uint renderID;
    uint meshDataIndex;
    uint padding[2];
};

layout (std430, set = 0, binding = 1) readonly buffer StandardSSBOBuffer {
    StandardSSBO ssbo[];
};

layout (std430, set = 0, binding = 2) readonly buffer MetaBuffer {
    RawDataMetaData rawDataTable[];
};

layout (std430, set = 0, binding = 3) readonly buffer AllDataBuffer {
    uint rawData[];
};

mat4 RawDataLoader_LoadMat4(uint base) {
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

vec2 RawDataLoader_LoadVec2(uint base) {
    return vec2(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1])
    );
}

vec3 RawDataLoader_LoadVec3(uint base) {
    return vec3(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1]),
        uintBitsToFloat(rawData[base + 2])
    );
}

vec4 RawDataLoader_LoadVec4(uint base) {
    return vec4(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1]),
        uintBitsToFloat(rawData[base + 2]),
        uintBitsToFloat(rawData[base + 3])
    );
}

uint RawDataLoader_LoadUint(uint base) {
    return rawData[base];
}

int RawDataLoader_LoadInt(uint base) {
    return int(rawData[base]);
}

RawDataMetaData RawDataLoader_GetRawDataMetaData(uint index) {
    return rawDataTable[nonuniformEXT(index)];
}

StandardSSBO RawDataLoader_LoadStandardSSBO(uint index) {
    return ssbo[nonuniformEXT(index)];
}
#endif