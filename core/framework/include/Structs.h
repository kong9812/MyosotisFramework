// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Buffer.h"
#include "ivma.h"
#include "iglm.h"

namespace MyosotisFW
{
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
		glm::mat4 model;
		glm::vec4 position;
		glm::vec4 rotation;
		glm::vec4 scale;
		glm::vec4 color;
		uint32_t renderID;
		uint32_t meshDataIndex;
		uint32_t padding[2];
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
