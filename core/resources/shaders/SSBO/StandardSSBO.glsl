#ifndef STANDARDSSBO
#define STANDARDSSBO
#include "../Loader/RawDataLoader.glsl"

struct StandardSSBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    uint renderID;
};

const uint StandardSSBOSize = 53;

StandardSSBO LoadStandardSSBO(uint offset)
{
    StandardSSBO ssbo;
    ssbo.model = LoadMat4(offset + 0);
    ssbo.view = LoadMat4(offset + 16);
    ssbo.projection = LoadMat4(offset + 32);
    ssbo.color = LoadVec4(offset + 48);
    ssbo.renderID = LoadUint(offset + 52);
    return ssbo;
}
#endif