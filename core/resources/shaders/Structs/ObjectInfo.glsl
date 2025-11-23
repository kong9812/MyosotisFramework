#ifndef OBJECTINFO
#define OBJECTINFO
#include "Transform.glsl"

struct ObjectInfo {
  Transform transform;
  uint objectID;
  uint meshID;
  uint materialID;
  uint _padding;
};

layout (std430, set = 1, binding = 0) readonly buffer ObjectInfoDescriptor {
  ObjectInfo objectInfoRaw[];
};

ObjectInfo ObjectInfo_GetObjectInfo(uint index)
{
  return objectInfoRaw[index];
}

#endif