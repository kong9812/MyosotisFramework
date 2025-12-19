#ifndef CAMERAINFO
#define CAMERAINFO

struct CameraData {
  vec4 frustumPlanes[6];
  mat4 view;
  mat4 projection;
  mat4 invView;
  mat4 invProjection;
  vec4 pos;
};

layout (std430, set = 0, binding = 2) readonly buffer CameraInfoDescriptor {
  uint mainCameraIndex;
  uint _p1;
  uint _p2;
  uint _p3;
  CameraData cameraData[];
};

uint CameraInfo_GetMainCameraIndex()
{
  return mainCameraIndex;
}

CameraData CameraInfo_GetCameraData(uint index)
{
  return cameraData[index];
}

#endif