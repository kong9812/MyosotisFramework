// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshInfo
	{
		glm::vec4 AABBMin = glm::vec4(0.0f);			// AABBの最小値
		glm::vec4 AABBMax = glm::vec4(0.0f);			// AABBの最大値
		glm::ivec2 atlasSize = glm::ivec2(0);		// アトラスサイズ
		uint32_t meshID = 0;						// MeshID
		uint32_t meshletInfoOffset = 0;				// [DescriptorData]MeshletMetaDataの開始位置
		uint32_t meshletCount = 0;					// MeshletMetaDataの数
		uint32_t vertexAttributeBit = 0;			// [仮* DescriptorData]頂点属性のビットフラグ
		uint32_t unitSize = 0;						// [仮* DescriptorData]一枚当たりのサイズ
		uint32_t vertexFloatCount = 0;				// [仮*] 頂点Floatデータの数
		uint32_t indexCount = 0;					// Indexの数
		uint32_t vertexDataOffset = 0;
		uint32_t indexDataOffset = 0;
		uint32_t materialID = 0;					// MaterialID
	};
	// *ObjectData: Objectでセットするデータ
	// *DescriptorData: DescriptorSetでセットするデータ
	// todo. vertexAttributeBitとunitSizeはロード時セットするように変更
	// todo. vertexFloatCountはfloat数ではなく頂点数に変更
}
