#ifndef DIRECTIONALLIGHTSSBO
#define DIRECTIONALLIGHTSSBO
#include "../Loader/RawDataLoader.glsl"

struct DirectionalLightSSBO {
	mat4 viewProjection;
	vec4 position;
	int pcfCount;
};

const uint DirectionalLightSSBOSize = 21;

DirectionalLightSSBO DirectionalLightSSBO_LoadDirectionalLightSSBO(uint offset) {
    DirectionalLightSSBO ssbo;
    ssbo.viewProjection = RawDataLoader_LoadMat4(offset + 0);
    ssbo.position = RawDataLoader_LoadVec4(offset + 16);
    ssbo.pcfCount = RawDataLoader_LoadInt(offset + 20);
    return ssbo;
}
#endif