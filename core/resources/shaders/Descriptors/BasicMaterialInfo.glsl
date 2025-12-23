#ifndef BASICMATERIALINFO
#define BASICMATERIALINFO

struct BasicMaterialInfo {
	uint bitFlags;
	uint baseColorTexture;
	uint normalTexture;
	uint p3;
	vec4 baseColor;
};

layout (std430, set = 5, binding = 0) readonly buffer BasicMaterialInfoDescriptor {
	BasicMaterialInfo basicMaterialInfo[];
};

bool BasicMaterialInfo_HasBaseColorTexture(uint bitFlags)
{
	return (bitFlags & (1u << 0)) != 0u;
}

BasicMaterialInfo BasicMaterialInfo_GetBasicMaterialInfo(uint index)
{
	return basicMaterialInfo[index];
}

#endif