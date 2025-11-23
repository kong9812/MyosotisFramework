// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

namespace MyosotisFW
{
	// VB: VisibilityBuffer
	struct VBDispatchInfo
	{
		// 描画直前にセット
		uint32_t objectID;

		// DescriptorにMeshDataを追加するときに取得
		uint32_t meshID;

		// ここは0からスタートし、GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
		uint32_t meshletID;
	};
}