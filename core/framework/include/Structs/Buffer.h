// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace MyosotisFW
{
	struct Buffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
		VkDescriptorBufferInfo descriptor{};

		size_t localSize = 0;	// アライメント関係上 実際使うサイズとallocationInfoのサイズが異なる時がある
		bool needFlush = false;
	};
}
