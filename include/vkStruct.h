#pragma once
#include <vulkan/vulkan.h>

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
}