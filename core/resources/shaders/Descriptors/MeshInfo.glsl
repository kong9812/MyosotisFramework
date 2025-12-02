#ifndef MESHINFO
#define MESHINFO

struct MeshInfo {
	vec4 AABBMin;				// AABBの最小値
  	vec4 AABBMax;				// AABBの最大値
  	uint meshID;				// MeshID
  	uint meshletInfoOffset;		// MeshletMetaDataの開始位置
  	uint meshletCount;			// MeshletMetaDataの数
  	uint vertexAttributeBit;  	// 頂点属性のビットフラグ
	uint unitSize;            	// 一枚当たりのサイズ
  	uint _p1;					// 
  	uint _p2;					// 
  	uint _p3;					// 
};

layout (std430, set = 2, binding = 0) readonly buffer MeshInfoDescriptor {
	MeshInfo meshInfo[];
};

MeshInfo MeshInfo_GetMeshInfo(uint index)
{
	return meshInfo[index];
}

#endif