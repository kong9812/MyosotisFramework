// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	class LightingRenderPipeline : public RenderPipelineBase
	{
	public:
		LightingRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device),
			m_lightingShaderObject{},
			m_positionDescriptorImageInfo{},
			m_normalDescriptorImageInfo{},
			m_baseColorDescriptorImageInfo{} {
		}
		~LightingRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void UpdateDirectionalLightInfo(const DirectionalLightInfo& lightInfo);
		void UpdateCameraPosition(const glm::vec4& position);
		void CreateShaderObject(const VkDescriptorImageInfo& shadowMapImageInfo);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		LightingShaderObject m_lightingShaderObject;

		VkDescriptorImageInfo m_positionDescriptorImageInfo;
		VkDescriptorImageInfo m_normalDescriptorImageInfo;
		VkDescriptorImageInfo m_baseColorDescriptorImageInfo;

	};
	TYPEDEF_UNIQUE_PTR_ARGS(LightingRenderPipeline)
}