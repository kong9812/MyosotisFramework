// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "Buffer.h"

namespace MyosotisFW
{
	struct Descriptor
	{
		VkDescriptorType descriptorType;
		VkShaderStageFlagBits shaderStageFlagBits;
		VkDescriptorBindingFlags descriptorBindingFlags;
		int32_t descriptorCount;

		Buffer buffer;

		bool rebuild;
		bool update;
	};
}
