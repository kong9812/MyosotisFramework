#ifndef OBJECTINFO
#define OBJECTINFO
#include "Transform.glsl"

struct ObjectInfo {
  Transform transform;
  uint meshID;
  uint materialID;
};

// 生データ
struct ObjectInfoRaw {
  TransformRaw transform;
  uint meshID;
  uint materialID;
};

layout (std430, set = 1, binding = 0) readonly buffer ObjectInfoDescriptor {
  ObjectInfoRaw objectInfoRaw[];
};

ObjectInfo ObjectInfo_ToObjectInfo(ObjectInfoRaw rawData)
{
  ObjectInfo result;
  result.transform = Transform_ToTransform(rawData.transform);
  result.meshID = rawData.meshID;
  result.materialID = rawData.materialID;
  return result;
}

ObjectInfo ObjectInfo_GetObjectInfo(uint index)
{
  return ObjectInfo_ToObjectInfo(objectInfoRaw[index]);
}

#endif