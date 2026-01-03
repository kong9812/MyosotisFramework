// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshletInfo
	{
		glm::vec4 AABBMin = glm::vec4(0.0f);	// AABBの最小値
		glm::vec4 AABBMax = glm::vec4(0.0f);	// AABBの最大値
		uint32_t meshID = 0;					// [DescriptorData]MeshID
		uint32_t vertexCount = 0;				// 頂点の数
		uint32_t primitiveCount = 0;			// 三角形単位(三角形の数)
		uint32_t vertexDataOffset = 0;			// [DescriptorData]VertexDataの開始位置
		uint32_t uniqueIndexOffset = 0;			// [DescriptorData]UniqueIndexの開始位置
		uint32_t primitivesOffset = 0;			// [DescriptorData]Primitivesの開始位置
		uint32_t _p1 = 0;						// 
		uint32_t _p2 = 0;						// 
	};
	// *DescriptorData: DescriptorSetでセットするデータ
}
