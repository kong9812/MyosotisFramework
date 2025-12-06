#ifndef SCENEINFO
#define SCENEINFO

struct SceneInfo {
  uint _p1;
  uint _p2;
  uint _p3;
  uint _p4;
};

layout (std430, set = 0, binding = 1) readonly buffer SceneInfoDescriptor {
  SceneInfo sceneInfo[];
};

SceneInfo SceneInfo_GetSceneInfo(uint index)
{
  return sceneInfo[index];
}

#endif