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
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	}Transform;

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
		glm::vec2 screenSize;
	}UpdateData;

	typedef struct
	{
		VkImage image;
		VkImageView view;
		VkSampler sampler;
	}Image;

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
		VkSampler sampler;
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
		glm::vec3 min;
		glm::vec3 max;
	}Mesh;

	typedef struct
	{
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
	}ShaderBase;


	typedef struct
	{
		glm::mat4 viewProjection;
		glm::vec4 position;
		int32_t pcfCount;
	}DirectionalLightSSBO;

	typedef struct
	{
		glm::vec4 position;
	}CameraSSBO;

	typedef struct
	{
		glm::vec4 min;
		glm::vec4 max;
	}AABBData;

	typedef struct
	{
		glm::vec4 center;
		glm::vec4 axisX;
		glm::vec4 axisY;
		glm::vec4 axisZ;
	}OBBData;

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
	}FrustumCullersShaderObject;

	typedef struct
	{
		DirectionalLightSSBO lightSSBO;
	}ShadowMapShaderObject;

	typedef struct
	{
		ShaderBase shaderBase;

		CameraSSBO cameraSSBO;
		DirectionalLightSSBO lightSSBO;
	}LightingShaderObject;

	typedef struct
	{
		ShaderBase shadowMapRenderShaderBase;
		ShaderBase deferredRenderShaderBase;

		bool useNormalMap;
		Image normalMap;
		VkDescriptorImageInfo shadowMapImageInfo;

		struct {
			uint32_t objectIndex;
			uint32_t textureId;
		}standardPushConstant;

		struct {
			uint32_t objectIndex;
			uint32_t textureId;
		}shadowPushConstant;

		struct
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec4 color;
			uint32_t renderID;
		}standardSSBO;
	}StaticMeshShaderObject;

	typedef struct
	{
		ShaderBase shaderBase;
		Image cubeMap;

		struct {
			uint32_t objectIndex;
			uint32_t textureId;
		}pushConstant;

		struct
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec4 color;
			uint32_t renderID;
		}standardSSBO;
	}SkyboxShaderObject;

	typedef struct
	{
		ShaderBase shaderBase;

		CameraSSBO cameraSSBO;

		struct
		{
			Image cubemap;
			Buffer buffer;
			struct
			{
				glm::mat4 model;
				glm::mat4 view;
				glm::mat4 projection;
				glm::vec4 color;
				uint32_t renderID;
			}data;
		}standardUBO;			// UBO
	}InteriorObjectShaderObject;

	typedef struct
	{
		std::string m_meshPath;
	}CustomMeshInfo;

	typedef struct
	{
		uint32_t typeID;
		uint32_t dataOffset;
	}MetaData;
}
