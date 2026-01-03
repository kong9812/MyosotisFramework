// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "AccelerationStructure.h"
#include "Buffer.h"

namespace MyosotisFW
{
	struct TLASInfo
	{
		Buffer instanceBuffer{};
		VkDeviceAddress instanceAddress{};

		VkAccelerationStructureGeometryKHR geometry{};
		VkAccelerationStructureBuildRangeInfoKHR buildRange{};
		VkAccelerationStructureBuildSizesInfoKHR buildSize{};

		AccelerationStructure tlas{};	// 本体

		uint32_t instanceCount = 0;
		bool allowUpdate = false;
		bool dirty = false;
	};
}
