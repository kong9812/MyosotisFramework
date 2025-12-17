// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshInfo
	{
		glm::vec4 AABBMin;				// AABBの最小値
		glm::vec4 AABBMax;				// AABBの最大値
		glm::ivec2 atlasSize;			// アトラスサイズ
		uint32_t meshID;				// [ObjectData]MeshID
		uint32_t meshletInfoOffset;		// [DescriptorData]MeshletMetaDataの開始位置
		uint32_t meshletCount;			// MeshletMetaDataの数
		uint32_t vertexAttributeBit;	// [仮* DescriptorData]頂点属性のビットフラグ
		uint32_t unitSize;				// [仮* DescriptorData]一枚当たりのサイズ
		uint32_t vertexFloatCount;		// [仮*] 頂点Floatデータの数
		uint32_t indexCount;			// Indexの数
		uint32_t p2;
		uint32_t p3;
		uint32_t p4;
	};
	// *ObjectData: Objectでセットするデータ
	// *DescriptorData: DescriptorSetでセットするデータ
	// todo. vertexAttributeBitとunitSizeはロード時セットするように変更
	// todo. vertexFloatCountはfloat数ではなく頂点数に変更
}
