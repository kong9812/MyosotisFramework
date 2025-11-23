// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshInfo
	{
		glm::vec3 AABBMin;				// AABBの最小値
		glm::vec3 AABBMax;				// AABBの最大値
		uint32_t meshID;				// MeshID
		uint32_t meshletInfoOffset;		// MeshletMetaDataの開始位置
		uint32_t meshletCount;			// MeshletMetaDataの数
	};
}
