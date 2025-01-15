// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

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
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
		void* mapped;
	}Buffer;

	typedef struct
	{
		glm::vec4 position;
		glm::vec4 color;
	}Vertex;

	typedef struct
	{
		std::vector<Vertex> vertex;
		std::vector<glm::vec3> index;
	}VertexAndIndex;

	typedef struct
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 color;
	}StaticMeshStandardUBO;
}
