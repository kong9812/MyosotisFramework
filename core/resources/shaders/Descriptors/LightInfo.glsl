#ifndef LIGHTINFO
#define LIGHTINFO

struct LightInfo {
    vec3 position;
    float intensity;

    vec3 direction;
    int type;

	vec3 color;
    float range;

	vec2 spotCos;
};

layout (std430, set = 0, binding = 3) readonly buffer LightInfoDescriptor {
	LightInfo lightInfo[];
};

LightInfo LightInfo_GetLightInfo(uint index)
{
	return lightInfo[index];
}

#endif