// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ivma.h"
#include "iglm.h"

namespace MyosotisFW
{
	struct Transform
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	};

	struct RenderPass
	{
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> framebuffer;
	};

	struct UpdateData
	{
		bool pause;
		float deltaTime;
		std::unordered_map<int, int> keyActions;
		std::unordered_map<int, int> mouseButtonActions;
		glm::vec2 mousePos;
		glm::vec2 screenSize;
	};

	struct Image
	{
		VkImage image;
		VkImageView view;
		VkSampler sampler;
	};

	struct DeviceImage
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	struct VMAImage
	{
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
	};

	struct Buffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkDescriptorBufferInfo descriptor;
	};

	struct Meshlet
	{
		std::vector<uint32_t> uniqueIndex;
		std::vector<uint32_t> primitives;
		glm::vec3 min;
		glm::vec3 max;
	};

	struct Mesh
	{
		std::vector<float> vertex;
		std::vector<Meshlet> meshlet;
		glm::vec3 min;
		glm::vec3 max;
	};

	struct ShaderBase
	{
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
	};

	struct DirectionalLightSSBO
	{
		glm::mat4 viewProjection;
		glm::vec4 position;
		int32_t pcfCount;
	};

	struct CameraData
	{
		glm::vec4 frustumPlanes[6];
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 position;
	};

	struct AABBData
	{
		glm::vec4 min;
		glm::vec4 max;
	};

	struct OBBData
	{
		glm::vec4 center;
		glm::vec4 axisX;
		glm::vec4 axisY;
		glm::vec4 axisZ;
	};

	struct StandardSSBO
	{
		OBBData obbData;
		glm::mat4 model;
		glm::vec4 color;
		uint32_t renderID;
		uint32_t vertexMetaIndex;
	};

	struct FrustumCullingShaderObject
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
				std::vector<OBBData> obbDatas;
			}data;
		}obbDatasSSBO;						// SSBO

		struct
		{
			Buffer buffer;
			struct
			{
				std::vector<uint32_t> visibleIndices;	// 可視オブジェクトのインデックス
			}data;
		}visibleObjectsSSBO;							// SSBO
	};

	struct CustomMeshInfo
	{
		std::string meshName;
	};

	struct MetaData
	{
		uint32_t typeID;
		uint32_t dataOffset;
	};
}
