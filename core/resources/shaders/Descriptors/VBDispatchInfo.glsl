#ifndef VBDISPATCHINFO
#define VBDISPATCHINFO

struct VBDispatchInfo {
  uint bitFlags;
  uint objectID;
  uint meshID;
  uint meshletID;
};

layout (std430, set = 1, binding = 1) readonly buffer VBDispatchInfoDescriptor {
  VBDispatchInfo vbDispatchInfo[];
};

bool VBDispatchInfo_IsCullingObject(uint bitFlags)
{
  return (bitFlags & 0x1u) == 0u;
}

VBDispatchInfo VBDispatchInfo_GetVBDispatchInfo(uint index)
{
  return vbDispatchInfo[index];
}

#endif