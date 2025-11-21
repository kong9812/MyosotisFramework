#ifndef TRANSFORM
#define TRANSFORM

struct Transform {
  vec3 pos;
  vec3 rot;
  vec3 scale;
};

// 生データ
struct TransformRaw {
  vec4 v1;
  vec4 v2;
  float v3;
};

Transform Transform_ToTransform(TransformRaw rawData)
{
  Transform result;
  result.pos = vec3(rawData.v1.x, rawData.v1.y, rawData.v1.z);
  result.rot = vec3(rawData.v1.w, rawData.v2.x, rawData.v2.y);
  result.scale = vec3(rawData.v2.z, rawData.v2.w, rawData.v3);
  return result;
}

#endif