// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "iglm.h"

namespace MyosotisFW
{
	struct TLASInstanceInfo
	{
		uint32_t objectID;
		uint32_t blasID;

		glm::mat4 transform;		// 差分比較用

		uint8_t  mask;				// VkAccelerationStructureInstanceKHR::mask で使う当たるかどうかを判定用フィルタ
		uint32_t hitGroupOffset;	// どのHitGroupを使うかのオフセット 0透明 1不透明 2Hairとか 状況に応じてセット (今は1しかないけどね…)

		uint32_t instanceCustomIndex;

		bool transformDirty;
		bool structureDirty;
	};
}
