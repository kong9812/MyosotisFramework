#ifndef MESHLETINFO
#define MESHLETINFO

struct MeshletInfo {
		vec4 AABBMin;			      // AABBの最小値
		vec4 AABBMax;			      // AABBの最大値
		uint meshID;			      // MeshID
		uint vertexCount;		    // 頂点の数
		uint primitiveCount;    // 三角形単位(三角形の数)
		uint vertexAttributeBit;// 頂点属性のビットフラグ
		uint unitSize;          // 一枚当たりのサイズ
		uint vertexDataOffset;	// VertexDataの開始位置
		uint uniqueIndexOffset;	// UniqueIndexの開始位置
		uint primitivesOffset;	// Primitivesの開始位置
};

layout (std430, set = 2, binding = 1) readonly buffer MeshletInfoDescriptor {
  MeshletInfo meshletInfo[];
};

MeshletInfo MeshletInfo_GetMeshletInfo(uint index)
{
  return meshletInfo[index];
}

#endif