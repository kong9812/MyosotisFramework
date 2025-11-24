#ifndef UNIQUEINDEXDATA
#define UNIQUEINDEXDATA

layout (std430, set = 2, binding = 3) readonly buffer UniqueIndexDataDescriptor {
  uint uniqueIndexData[];
};

uint UniqueIndexData_GetUniqueIndexData(uint uniqueIndexOffset, uint index)
{
    return uniqueIndexData[uniqueIndexOffset + index];
}

#endif