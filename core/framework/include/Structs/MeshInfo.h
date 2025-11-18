// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct MeshInfo
	{
		glm::vec4 AABBMin;				// AABBの最小値
		glm::vec4 AABBMax;				// AABBの最大値
		uint32_t meshID;				// MeshID
		uint32_t meshletInfoOffset;	// MeshletMetaDataの開始位置
		uint32_t meshletCount;	// MeshletMetaDataの数
		uint32_t empty;					// 予約領域(将来の拡張用)
	};
}
