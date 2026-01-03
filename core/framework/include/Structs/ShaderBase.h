// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

namespace MyosotisFW
{
	struct ShaderBase
	{
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
	};
}