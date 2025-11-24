#ifndef PRIMITIVESDATA
#define PRIMITIVESDATA

layout (std430, set = 2, binding = 4) readonly buffer PrimitivesDataDescriptor {
  uint primitivesData[];
};

uvec3 PrimitivesData_LoadVec3(uint base) 
{
  return uvec3(
    primitivesData[base + 0],
    primitivesData[base + 1],
    primitivesData[base + 2]);
}

uvec3 PrimitivesData_GetPrimitivesData(uint primitivesOffset, uint index)
{
  return PrimitivesData_LoadVec3(primitivesOffset + (index * 3));
}

#endif