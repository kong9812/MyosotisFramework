#ifndef BASICMATERIALINFO
#define BASICMATERIALINFO

struct BasicMaterialInfo {
	uint bitFlags;
	uint baseColorTexture;
	uint normalTexture;
	uint p3;
	vec4 baseColor;
};

layout (std430, set = 0, binding = 1) readonly buffer BasicMaterialInfoDescriptor {
  BasicMaterialInfo basicMaterialInfo[];
};

BasicMaterialInfo BasicMaterialInfo_GetBasicMaterialInfo(uint index)
{
  return basicMaterialInfo[index];
}

#endif