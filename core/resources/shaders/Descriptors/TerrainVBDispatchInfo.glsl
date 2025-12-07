#ifndef TERRAINVBDISPATCHINFO
#define TERRAINVBDISPATCHINFO

struct VBDispatchInfo {
  uint bitFlags;
  uint objectID;
  uint meshID;
  uint meshletID;
};

layout (std430, set = 0, binding = 5) readonly buffer TerrainVBDispatchInfoDescriptor {
  VBDispatchInfo terrainVBDispatchInfo[];
};

bool TerrainVBDispatchInfo_IsCullingObject(uint bitFlags)
{
  return (bitFlags & 0x1u) == 0u;
}

VBDispatchInfo TerrainVBDispatchInfo_GetVBDispatchInfo(uint index)
{
  return terrainVBDispatchInfo[index];
}

#endif