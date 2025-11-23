#ifndef VBDISPATCHINFO
#define VBDISPATCHINFO

struct VBDispatchInfo {
  uint objectID;
  uint meshID;
  uint meshletID;
};

layout (std430, set = 1, binding = 1) readonly buffer VBDispatchInfoDescriptor {
  VBDispatchInfo vbDispatchInfo[];
};

VBDispatchInfo VBDispatchInfo_GetVBDispatchInfo(uint index)
{
  return vbDispatchInfo[index];
}

#endif