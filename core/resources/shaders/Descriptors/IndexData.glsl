#ifndef INDEXDATA
#define INDEXDATA

layout (std430, set = 2, binding = 5) readonly buffer IndexDataDescriptor {
  uint indexData[];
};

uint IndexData_GetIndexData(uint offset, uint index)
{
    return indexData[offset + index];
}

#endif