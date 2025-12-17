// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "AccelerationStructure.h"
#include "Buffer.h"

namespace MyosotisFW
{
	struct BLASInfo
	{
		Buffer vertexBuffer;
		Buffer indexBuffer;
		Buffer transformBuffer;

		VkDeviceAddress vertexAddress;
		VkDeviceAddress indexAddress;
		VkDeviceAddress transformAddress;

		VkAccelerationStructureGeometryKHR geometry;
		VkAccelerationStructureBuildRangeInfoKHR buildRange;
		VkAccelerationStructureBuildSizesInfoKHR buildSize;

		AccelerationStructure blas;	// 本体
		bool dirty;
	};
}
