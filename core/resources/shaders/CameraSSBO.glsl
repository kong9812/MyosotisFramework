#ifndef CAMERASSBO
#define CAMERASSBO
#include "RawDataLoader.glsl"

struct CameraSSBO {
    vec4 position;
};

const uint CameraSSBOSize = 4;

CameraSSBO LoadCameraSSBO(uint offset)
{
    CameraSSBO ssbo;
    ssbo.position = LoadVec4(offset + 0);
    return ssbo;
}
#endif