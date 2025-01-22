// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include "vma.h"

namespace Utility::Vulkan::Struct
{
	typedef struct
	{
		float deltaTime;
		std::unordered_map<int, int> keyActions;
		glm::vec2 mousePos;
	}UpdateData;

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
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkDescriptorBufferInfo descriptor;
	}Buffer;

	typedef struct
	{
		std::vector<float> vertex;
		std::vector<uint32_t> index;
	}Vertex;

	typedef struct
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
	}StaticMeshStandardUBO;
}
