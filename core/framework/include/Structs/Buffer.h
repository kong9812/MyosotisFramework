// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ivma.h"

namespace MyosotisFW
{
	struct Buffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkDescriptorBufferInfo descriptor;
	};
}
