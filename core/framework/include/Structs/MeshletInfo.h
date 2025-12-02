// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshletInfo
	{
		glm::vec4 AABBMin;			// AABBの最小値
		glm::vec4 AABBMax;			// AABBの最大値
		uint32_t meshID;			// MeshID
		uint32_t vertexCount;		// 頂点の数
		uint32_t primitiveCount;    // 三角形単位(三角形の数)
		uint32_t vertexDataOffset;	// VertexDataの開始位置
		uint32_t uniqueIndexOffset;	// UniqueIndexの開始位置
		uint32_t primitivesOffset;	// Primitivesの開始位置
		uint32_t _p1;				// 
		uint32_t _p2;				// 
	};
}
