// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class LightingRenderPipeline : public RenderPipelineBase
	{
	public:
		LightingRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors),
			m_descriptorSetLayout(VK_NULL_HANDLE),
			m_descriptorSet(VK_NULL_HANDLE),
			m_lightingShaderObject({}),
			m_positionDescriptorImageInfo({}),
			m_normalDescriptorImageInfo({}),
			m_baseColorDescriptorImageInfo({}) {
		}
		~LightingRenderPipeline();

		// ShaderObject
		struct ShaderObject
		{
			struct {
				uint32_t objectIndex;
				uint32_t textureId;
			}pushConstant;

			struct {
				DirectionalLightSSBO lightSSBO;
			}SSBO;
		};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void UpdateDirectionalLightInfo(const DirectionalLightSSBO& lightInfo);
		void CreateShaderObject(const VkDescriptorImageInfo& shadowMapImageInfo);
		void UpdateDescriptors(const VkDescriptorImageInfo& shadowMapImageInfo);


	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;

		ShaderObject m_lightingShaderObject;

		VkDescriptorImageInfo m_positionDescriptorImageInfo;
		VkDescriptorImageInfo m_normalDescriptorImageInfo;
		VkDescriptorImageInfo m_baseColorDescriptorImageInfo;

	};
	TYPEDEF_SHARED_PTR_ARGS(LightingRenderPipeline);
}