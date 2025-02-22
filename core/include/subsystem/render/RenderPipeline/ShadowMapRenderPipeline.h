// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class ShadowMapRenderPipeline : public RenderPipelineBase
	{
	public:
		ShadowMapRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VMAImage shadowMap);
		~ShadowMapRenderPipeline();

		void CreateShaderObject(StaticMeshShaderObject& shaderObject);

		DirectionalLightInfo GetDirectionalLightInfo();

		VkDescriptorImageInfo GetShadowMapDescriptorImageInfo() { return m_shadowMapDescriptorImageInfo; }

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;

		ShadowMapShaderObject m_shadowMapShaderObject;
		VkSampler m_shadowMapSampler;
		VkDescriptorImageInfo m_shadowMapDescriptorImageInfo;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(ShadowMapRenderPipeline)
}