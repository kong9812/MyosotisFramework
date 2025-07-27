#ifndef STANDARDSSBO
#define STANDARDSSBO
#include "../Loader/RawDataLoader.glsl"

struct StandardSSBO {
    mat4 model;
    vec4 color;
    uint renderID;
};

const uint StandardSSBOSize = 53;

StandardSSBO StandardSSBO_LoadStandardSSBO(uint offset) {
    StandardSSBO ssbo;
    ssbo.model = RawDataLoader_LoadMat4(offset + 0);
    ssbo.color = RawDataLoader_LoadVec4(offset + 48);
    ssbo.renderID = RawDataLoader_LoadUint(offset + 52);
    return ssbo;
}
#endif