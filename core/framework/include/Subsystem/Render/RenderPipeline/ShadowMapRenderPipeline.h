// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class ShadowMapRenderPipeline : public RenderPipelineBase
	{
	public:
		ShadowMapRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device) {
		}
		~ShadowMapRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(StaticMeshShaderObject& shaderObject);
		void UpdateDescriptors(StaticMeshShaderObject& shaderObject);
		void Resize(const RenderResources_ptr& resources) override;

		DirectionalLightInfo GetDirectionalLightInfo();

		VkDescriptorImageInfo GetShadowMapDescriptorImageInfo() { return m_shadowMapDescriptorImageInfo; }

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		ShadowMapShaderObject m_shadowMapShaderObject;
		VkSampler m_shadowMapSampler;
		VkDescriptorImageInfo m_shadowMapDescriptorImageInfo;
	};
	TYPEDEF_SHARED_PTR_ARGS(ShadowMapRenderPipeline);
}