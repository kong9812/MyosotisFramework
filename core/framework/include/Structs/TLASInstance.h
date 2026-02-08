// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "iglm.h"

namespace MyosotisFW
{
	struct TLASInstance
	{
		uint32_t objectID = 0;
		std::vector<uint32_t> meshID{};

		bool active()
		{
			return !meshID.empty();
		}

		glm::mat4 model = glm::mat4(0.0f);		// 差分比較用

		uint8_t  mask = 0;						// VkAccelerationStructureInstanceKHR::mask で使う当たるかどうかを判定用フィルタ
		uint32_t hitGroupOffset = 0;			// どのHitGroupを使うかのオフセット 0透明 1不透明 2Hairとか 状況に応じてセット (今は1しかないけどね…)

		bool transformDirty = false;
		bool structureDirty = false;
	};
	TYPEDEF_SHARED_PTR(TLASInstance);
}
