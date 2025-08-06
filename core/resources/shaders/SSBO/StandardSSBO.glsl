#ifndef STANDARDSSBO
#define STANDARDSSBO
#include "../Loader/RawDataLoader.glsl"

struct StandardSSBO {
    mat4 model;
    vec4 position;
    vec4 rotation;
    vec4 scale;
    vec4 color;
    uint renderID;
    uint meshDataIndex;
};

const uint StandardSSBOSize = 34;

StandardSSBO StandardSSBO_LoadStandardSSBO(uint offset) {
    StandardSSBO ssbo;
    ssbo.model = RawDataLoader_LoadMat4(offset + 0);
    ssbo.position = RawDataLoader_LoadVec4(offset + 16);
    ssbo.rotation = RawDataLoader_LoadVec4(offset + 20);
    ssbo.scale = RawDataLoader_LoadVec4(offset + 24);
    ssbo.color = RawDataLoader_LoadVec4(offset + 28);
    ssbo.renderID = RawDataLoader_LoadUint(offset + 32);
    ssbo.meshDataIndex = RawDataLoader_LoadUint(offset + 33);
    return ssbo;
}
#endif