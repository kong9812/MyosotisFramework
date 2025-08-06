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

struct MeshData {
    vec4 AABBMin;
    vec4 AABBMax;
    uint meshID;
    uint meshletMetaDataOffset;
    uint meshletMetaDataCount;
    uint empty;
};

struct MeshletMetaData {
	vec4 AABBMin;
    vec4 AABBMax;
    uint vertexCount;
	uint primitiveCount;
	uint vertexAttributeBit;
	uint unitSize;
	uint vertexDataOffset;
	uint uniqueIndexOffset;
	uint primitivesOffset;
	uint empty;
};

struct VertexData {
    vec4 position;
    vec3 normal;
    vec2 uv;
    vec4 color;
};

layout (std430, set = 1, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshDatas[];
};

layout (std430, set = 1, binding = 1) readonly buffer MeshletMetaDataBuffer {
    MeshletMetaData meshletMetaDatas[];
};

layout (std430, set = 1, binding = 2) readonly buffer AllVertexBuffer {
    uint vertexData[];
};

layout (std430, set = 1, binding = 3) readonly buffer AllUniqueIndexBuffer {
    uint uniqueIndexData[];
};

layout (std430, set = 1, binding = 4) readonly buffer AllPrimitivesBuffer {
    uint primitivesData[];
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

uvec3 VertexDataLoader_LoadPrimitiveUVec3(uint base) {
    return uvec3(
        primitivesData[base + 0],
        primitivesData[base + 1],
        primitivesData[base + 2]);
}

uint VertexDataLoader_LoadUniqueIndexDataUint(uint base) {
    return uniqueIndexData[base + 0];
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

MeshletMetaData VertexDataLoader_GetMeshletMetaData(uint meshID, uint meshletIndex)
{
    MeshData meshData = meshDatas[nonuniformEXT(meshID)];   
    return meshletMetaDatas[nonuniformEXT(meshData.meshletMetaDataOffset + meshletIndex)];
}

MeshData VertexDataLoader_GetMeshData(uint index)
{
    return meshDatas[nonuniformEXT(index)];
}

VertexData VertexDataLoader_GetVertexData(uint meshID, uint meshletIndex, uint vertexIndex) {
    MeshData meshData = meshDatas[nonuniformEXT(meshID)];
    MeshletMetaData meshletMetaData = meshletMetaDatas[nonuniformEXT(meshData.meshletMetaDataOffset + meshletIndex)];
    uint offset = meshletMetaData.vertexDataOffset + (meshletMetaData.unitSize * vertexIndex);
    VertexData v;

    if ((meshletMetaData.vertexAttributeBit & POSITION_VEC3) != 0) {
        v.position = vec4(VertexDataLoader_LoadVec3(offset), 0.0);
        offset += 3;
    } else if ((meshletMetaData.vertexAttributeBit & POSITION_VEC4) != 0) {
        v.position = VertexDataLoader_LoadVec4(offset);
        offset += 4;
    }

    if ((meshletMetaData.vertexAttributeBit & NORMAL) != 0) {
        v.normal = VertexDataLoader_LoadVec3(offset);
        offset += 3;
    }

    if ((meshletMetaData.vertexAttributeBit & UV) != 0) {
        v.uv = VertexDataLoader_LoadVec2(offset);
        offset += 2;
    }

    if ((meshletMetaData.vertexAttributeBit & COLOR_VEC3) != 0) {
        v.color = vec4(VertexDataLoader_LoadVec3(offset), 1.0);
        offset += 3;
    } else if ((meshletMetaData.vertexAttributeBit & COLOR_VEC4) != 0) {
        v.color = VertexDataLoader_LoadVec4(offset);
        offset += 4;
    }
    return v;
}

uvec3 VertexDataLoader_GetPrimitivesData(uint meshID, uint meshletIndex, uint primitiveIndex)
{
    MeshData meshData = meshDatas[nonuniformEXT(meshID)];
    MeshletMetaData meshletMetaData = meshletMetaDatas[nonuniformEXT(meshData.meshletMetaDataOffset + meshletIndex)];
    uint offset = meshletMetaData.primitivesOffset + (primitiveIndex * 3);
    return VertexDataLoader_LoadPrimitiveUVec3(offset);
}

uint VertexDataLoader_GetUniqueIndexData(uint meshID, uint meshletIndex, uint uniqueIndex)
{
    MeshData meshData = meshDatas[nonuniformEXT(meshID)];
    MeshletMetaData meshletMetaData = meshletMetaDatas[nonuniformEXT(meshData.meshletMetaDataOffset + meshletIndex)];
    uint offset = meshletMetaData.uniqueIndexOffset + uniqueIndex;
    return VertexDataLoader_LoadUniqueIndexDataUint(offset);
}
#endif