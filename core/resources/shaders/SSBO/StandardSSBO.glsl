#ifndef STANDARDSSBO
#define STANDARDSSBO
#include "../Loader/RawDataLoader.glsl"

struct StardardOBBData {
    vec4 center;
	vec4 axisX;
	vec4 axisY;
	vec4 axisZ;
};
struct StandardSSBO {
    StardardOBBData obbData;
    mat4 model;
    vec4 color;
    uint renderID;
    uint vertexMetaIndex;
};

const uint StandardSSBOSize = 35;

StandardSSBO StandardSSBO_LoadStandardSSBO(uint offset) {
    StandardSSBO ssbo;
    ssbo.obbData.center = RawDataLoader_LoadVec4(offset + 0);
    ssbo.obbData.axisX = RawDataLoader_LoadVec4(offset + 4);
    ssbo.obbData.axisY = RawDataLoader_LoadVec4(offset + 8);
    ssbo.obbData.axisZ = RawDataLoader_LoadVec4(offset + 12);
    ssbo.model = RawDataLoader_LoadMat4(offset + 16);
    ssbo.color = RawDataLoader_LoadVec4(offset + 32);
    ssbo.renderID = RawDataLoader_LoadUint(offset + 33);
    ssbo.vertexMetaIndex = RawDataLoader_LoadUint(offset + 34);
    return ssbo;
}
#endif