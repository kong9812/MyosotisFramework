#ifndef DIRECTIONALLIGHTSSBO
#define DIRECTIONALLIGHTSSBO
#include "../Loader/RawDataLoader.glsl"

struct DirectionalLightSSBO {
	mat4 viewProjection;
	vec4 position;
	int pcfCount;
};

const uint DirectionalLightSSBOSize = 21;

DirectionalLightSSBO LoadDirectionalLightSSBO(uint offset)
{
    DirectionalLightSSBO ssbo;
    ssbo.viewProjection = LoadMat4(offset + 0);
    ssbo.position = LoadVec4(offset + 16);
    ssbo.pcfCount = LoadInt(offset + 20);
    return ssbo;
}
#endif