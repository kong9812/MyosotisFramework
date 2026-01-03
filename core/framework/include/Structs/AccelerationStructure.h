// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "Buffer.h"

namespace MyosotisFW
{
	struct AccelerationStructure
	{
		Buffer buffer{};
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
		VkDeviceAddress deviceAddress{};
	};
}