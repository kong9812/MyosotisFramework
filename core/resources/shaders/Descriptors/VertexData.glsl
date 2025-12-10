#ifndef VERTEXDATA
#define VERTEXDATA

const uint POSITION_VEC3 = 0x00000001;
const uint POSITION_VEC4 = 0x00000002;
const uint NORMAL        = 0x00000004;
const uint UV0           = 0x00000008;
const uint UV1           = 0x00000010;
const uint TANGENT       = 0x00000020;
const uint COLOR_VEC3    = 0x00000040;
const uint COLOR_VEC4    = 0x00000080;

struct VertexData {
    vec4 position;
    vec3 normal;
    vec2 uv0;
    vec2 uv1;
    vec4 color;
};

layout (std430, set = 2, binding = 2) readonly buffer VertexDataDescriptor {
  float vertexData[];
};

mat4 VertexData_LoadMat4(uint base) {
    return mat4(
        vec4(vertexData[base +  0], vertexData[base +  1],
             vertexData[base +  2], vertexData[base +  3]),
        vec4(vertexData[base +  4], vertexData[base +  5],
             vertexData[base +  6], vertexData[base +  7]),
        vec4(vertexData[base +  8], vertexData[base +  9],
             vertexData[base + 10], vertexData[base + 11]),
        vec4(vertexData[base + 12], vertexData[base + 13],
             vertexData[base + 14], vertexData[base + 15])
    );
}

vec2 VertexData_LoadVec2(uint base) {
    return vec2(
        vertexData[base + 0],
        vertexData[base + 1]
    );
}

vec3 VertexData_LoadVec3(uint base) {
    return vec3(
        vertexData[base + 0],
        vertexData[base + 1],
        vertexData[base + 2]
    );
}

vec4 VertexData_LoadVec4(uint base) {
    return vec4(
        vertexData[base + 0],
        vertexData[base + 1],
        vertexData[base + 2],
        vertexData[base + 3]
    );
}

VertexData VertexData_GetVertexData(uint vertexDataOffset, uint vertexAttributeBit, uint unitSize, uint index) 
{
    uint offset = vertexDataOffset + (unitSize * index);
    VertexData v;

    if ((vertexAttributeBit & POSITION_VEC3) != 0) {
        v.position = vec4(VertexData_LoadVec3(offset), 1.0);
        offset += 3;
    } else if ((vertexAttributeBit & POSITION_VEC4) != 0) {
        v.position = VertexData_LoadVec4(offset);
        offset += 4;
    }

    if ((vertexAttributeBit & NORMAL) != 0) {
        v.normal = VertexData_LoadVec3(offset);
        offset += 3;
    }

    if ((vertexAttributeBit & UV0) != 0) {
        v.uv0 = VertexData_LoadVec2(offset);
        offset += 2;
    }

    if ((vertexAttributeBit & UV1) != 0) {
        v.uv1 = VertexData_LoadVec2(offset);
        offset += 2;
    }

    if ((vertexAttributeBit & COLOR_VEC3) != 0) {
        v.color = vec4(VertexData_LoadVec3(offset), 1.0);
        offset += 3;
    } else if ((vertexAttributeBit & COLOR_VEC4) != 0) {
        v.color = VertexData_LoadVec4(offset);
        offset += 4;
    }
    return v;
}

#endif