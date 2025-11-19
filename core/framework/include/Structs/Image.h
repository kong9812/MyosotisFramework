// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ivma.h"

namespace MyosotisFW
{
	struct Image
	{
		VkImage image;
		VkImageView view;
		std::vector<VkImageView> mipView;
		VkSampler sampler;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
	};
}
