// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include "ivma.h"
#include "iglm.h"

namespace MyosotisFW
{
	typedef struct
	{
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> framebuffer;
	}RenderPass;

	typedef struct
	{
		bool pause;
		float deltaTime;
		std::unordered_map<int, int> keyActions;
		std::unordered_map<int, int> mouseButtonActions;
		glm::vec2 mousePos;
	}UpdateData;

	typedef struct
	{
		VkImage image;
		VkImageView view;
	}Image;

	typedef struct
	{
		Image image;
		VkSampler sampler;
	}ImageWithSampler;

	typedef struct
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	}DeviceImage;

	typedef struct
	{
		VkImage image;
		VkImageView view;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
	}VMAImage;

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
	}Mesh;

	typedef struct
	{
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
	}ShaderBase;

	typedef struct
	{
		struct
		{
			VkDescriptorSet descriptorSet;
			VkPipelineLayout pipelineLayout;
			VkDescriptorSetLayout descriptorSetLayout;
			VkPipeline pipeline;
		}shaderBase;		// todo.

		struct
		{
			Buffer buffer;
			struct
			{
				glm::vec4 planes[6];	// 視錐台平面情報
			}data;
		}frustumPlanesUBO;				// UBO

		struct
		{
			Buffer buffer;
			struct
			{
				std::vector<glm::vec4> objects;	// (x, y, z, radius)
			}data;
		}objectDataSSBO;						// SSBO

		struct
		{
			Buffer buffer;
			struct
			{
				std::vector<uint32_t> visibleIndices;	// 可視オブジェクトのインデックス
			}data;
		}visibleObjectsSSBO;							// SSBO
	}FrustumCullersShaderObject;

	typedef struct
	{
		struct
		{
			Buffer buffer;
			struct
			{
				glm::mat4 viewProjection;
				glm::vec4 position;
				int32_t pcfCount;
			}data;
		}lightUBO;			// UBO
	}ShadowMapShaderObject;

	typedef struct
	{
		ShaderBase shadowMapRenderShaderBase;
		ShaderBase deferredRenderShaderBase;
		ShaderBase transparentRenderShaderBase;

		struct
		{
			ImageWithSampler normalMap;
			VkDescriptorImageInfo shadowMapImageInfo;
			VkDescriptorBufferInfo shadowMapBufferDescriptor;
			Buffer buffer;
			struct
			{
				glm::mat4 model;
				glm::mat4 view;
				glm::mat4 projection;
				glm::vec4 color;
				glm::vec4 cameraPos;
			}data;
		}standardUBO;			// UBO
	}StaticMeshShaderObject;


	typedef struct
	{
		std::string m_meshPath;
	}CustomMeshInfo;
}
