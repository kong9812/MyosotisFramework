// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ivma.h"

namespace MyosotisFW
{
	struct Image
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		std::vector<VkImageView> mipView{};
		VkSampler sampler = VK_NULL_HANDLE;
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
	};
}
