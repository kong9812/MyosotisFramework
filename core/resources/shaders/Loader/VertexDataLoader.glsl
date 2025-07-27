#ifndef VERTEXDATALOADER
#define VERTEXDATALOADER
#extension GL_EXT_nonuniform_qualifier : require

const uint POSITION_VEC3 = 0x00000001;
const uint POSITION_VEC4 = 0x00000002;
const uint NORMAL        = 0x00000004;
const uint UV            = 0x00000008;
const uint TANGENT       = 0x00000010;
const uint COLOR_VEC3    = 0x00000020;
const uint COLOR_VEC4    = 0x00000040;

// todo.これはMeshLet単位で分ける必要がある
struct VertexDataMetaData {
    uint vertexCount;
    uint primitiveCount;    // 三角形単位(三角形の数)
    uint vertexAttributeBit;
    uint unitSize;          // 一枚当たりのサイズ
    uint offset;
};

struct IndexDataMetaData {
    uint offset;            // IndexDataの開始位置
};

struct VertexData {
    vec4 position;
    vec3 normal;
    vec2 uv;
    vec4 color;
    vec4 tangent;
};

layout (std430, set = 0, binding = 5) readonly buffer VertexMetaBuffer {
    VertexDataMetaData vertexDataTable[];
};

layout (std430, set = 0, binding = 6) readonly buffer AllVertexBuffer {
    uint vertexData[];
};

layout (std430, set = 0, binding = 7) readonly buffer IndexMetaBuffer {
    IndexDataMetaData indexDataTable[];
};

layout (std430, set = 0, binding = 7) readonly buffer AllIndexBuffer {
    uint indexData[];
};

mat4 VertexDataLoader_LoadMat4(uint base) {
    return mat4(
        vec4(uintBitsToFloat(vertexData[base +  0]), uintBitsToFloat(vertexData[base +  1]),
             uintBitsToFloat(vertexData[base +  2]), uintBitsToFloat(vertexData[base +  3])),
        vec4(uintBitsToFloat(vertexData[base +  4]), uintBitsToFloat(vertexData[base +  5]),
             uintBitsToFloat(vertexData[base +  6]), uintBitsToFloat(vertexData[base +  7])),
        vec4(uintBitsToFloat(vertexData[base +  8]), uintBitsToFloat(vertexData[base +  9]),
             uintBitsToFloat(vertexData[base + 10]), uintBitsToFloat(vertexData[base + 11])),
        vec4(uintBitsToFloat(vertexData[base + 12]), uintBitsToFloat(vertexData[base + 13]),
             uintBitsToFloat(vertexData[base + 14]), uintBitsToFloat(vertexData[base + 15]))
    );
}

vec2 VertexDataLoader_LoadVec2(uint base) {
    return vec2(
        uintBitsToFloat(vertexData[base + 0]),
        uintBitsToFloat(vertexData[base + 1])
    );
}

vec3 VertexDataLoader_LoadVec3(uint base) {
    return vec3(
        uintBitsToFloat(vertexData[base + 0]),
        uintBitsToFloat(vertexData[base + 1]),
        uintBitsToFloat(vertexData[base + 2])
    );
}

uvec3 VertexDataLoader_LoadUVec3(uint base) {
    return uvec3(
        vertexData[base + 0],
        vertexData[base + 1],
        vertexData[base + 2]
    );
}

vec4 VertexDataLoader_LoadVec4(uint base) {
    return vec4(
        uintBitsToFloat(vertexData[base + 0]),
        uintBitsToFloat(vertexData[base + 1]),
        uintBitsToFloat(vertexData[base + 2]),
        uintBitsToFloat(vertexData[base + 3])
    );
}

uint VertexDataLoader_LoadUint(uint base) {
    return vertexData[base];
}

int VertexDataLoader_LoadInt(uint base) {
    return int(vertexData[base]);
}

VertexDataMetaData VertexDataLoader_GetVertexDataMetaData(uint index)
{
    return vertexDataTable[nonuniformEXT(index)];
}

uvec3 VertexDataLoader_GetIndexData(uint index, uint indexIndex)
{
    IndexDataMetaData meta = indexDataTable[nonuniformEXT(index)];
    uint offset = meta.offset + (3 * indexIndex);
    return VertexDataLoader_LoadUVec3(offset);
}

VertexData VertexDataLoader_GetVertexData(uint index, uint vertexIndex) {
    VertexDataMetaData meta = vertexDataTable[nonuniformEXT(index)];
    uint offset = meta.offset + (meta.unitSize * vertexIndex);
    VertexData v;

    if ((meta.vertexAttributeBit & POSITION_VEC3) != 0) {
        v.position = vec4(VertexDataLoader_LoadVec3(offset), 0.0);
        offset += 3;
    } else if ((meta.vertexAttributeBit & POSITION_VEC4) != 0) {
        v.position = VertexDataLoader_LoadVec4(offset);
        offset += 4;
    }

    if ((meta.vertexAttributeBit & NORMAL) != 0) {
        v.normal = VertexDataLoader_LoadVec3(offset);
        offset += 3;
    }

    if ((meta.vertexAttributeBit & UV) != 0) {
        v.uv = VertexDataLoader_LoadVec2(offset);
        offset += 2;
    }

    if ((meta.vertexAttributeBit & TANGENT) != 0) {
        v.tangent = VertexDataLoader_LoadVec4(offset);
        offset += 4;
    }

    if ((meta.vertexAttributeBit & COLOR_VEC3) != 0) {
        v.color = vec4(VertexDataLoader_LoadVec3(offset), 1.0);
        offset += 3;
    } else if ((meta.vertexAttributeBit & COLOR_VEC4) != 0) {
        v.color = VertexDataLoader_LoadVec4(offset);
        offset += 4;
    }
    return v;
}

#endif