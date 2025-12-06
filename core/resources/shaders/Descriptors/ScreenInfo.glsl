#ifndef SCREENINFO
#define SCREENINFO

struct ScreenInfo {
  ivec2 screenSize;
  uint _p1;
  uint _p2;
};

layout (std430, set = 0, binding = 0) readonly buffer ScreenInfoDescriptor {
  ScreenInfo screenInfo[];
};

ScreenInfo ScreenInfo_GetScreenInfo(uint index)
{
  return screenInfo[index];
}

#endif