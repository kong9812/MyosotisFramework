// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

namespace MyosotisFW
{
	// VB: VisibilityBuffer
	struct VBDispatchInfo
	{
		// BitFlags
		uint32_t bitFlags = 0;
		// 0: IsForceDraw(WithCulling)

		// 描画直前にセット
		uint32_t objectID = 0;

		// DescriptorにMeshDataを追加するときに取得
		uint32_t meshID = 0;

		// ここは0からスタートし、GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
		uint32_t meshletID = 0;
	};
}