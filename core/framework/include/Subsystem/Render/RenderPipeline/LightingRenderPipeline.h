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
		LightingRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device),
			m_lightingShaderObject({}),
			m_positionDescriptorImageInfo({}),
			m_normalDescriptorImageInfo({}),
			m_baseColorDescriptorImageInfo({}) {
		}
		~LightingRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void UpdateDirectionalLightInfo(const DirectionalLightInfo& lightInfo);
		void UpdateCameraPosition(const glm::vec4& position);
		void CreateShaderObject(const VkDescriptorImageInfo& shadowMapImageInfo);
		void UpdateDescriptors(const VkDescriptorImageInfo& shadowMapImageInfo);
		void Resize(const RenderResources_ptr& resources) override;

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		LightingShaderObject m_lightingShaderObject;

		VkDescriptorImageInfo m_positionDescriptorImageInfo;
		VkDescriptorImageInfo m_normalDescriptorImageInfo;
		VkDescriptorImageInfo m_baseColorDescriptorImageInfo;

	};
	TYPEDEF_SHARED_PTR_ARGS(LightingRenderPipeline);
}