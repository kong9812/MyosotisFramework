// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Utility::Vulkan::Struct
{
	typedef struct
	{
		VkImage image;
		VkImageView view;
	}Image;

	typedef struct
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	}DeviceImage;

	typedef struct
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
	}StaticMeshStandardUBO;
}
