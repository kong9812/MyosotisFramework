// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class ShadowMapRenderPipeline : public RenderPipelineBase
	{
	public:
		ShadowMapRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors),
			m_shadowMapDescriptorImageInfo({}) {
		}
		~ShadowMapRenderPipeline();

		// ShaderObject
		struct ShadowMapShaderObject
		{
			ShaderBase shaderBase;

			struct {
				uint32_t objectIndex;
				uint32_t textureId;
			}pushConstant;

			struct {
				StandardSSBO standardSSBO;
				DirectionalLightSSBO lightSSBO;
			}SSBO;
		};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(ShadowMapShaderObject& shaderObject);
		void UpdateDescriptors(ShadowMapShaderObject& shaderObject);

		DirectionalLightSSBO GetDirectionalLightInfo();
		VkDescriptorImageInfo GetShadowMapDescriptorImageInfo() { return m_shadowMapDescriptorImageInfo; }

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorImageInfo m_shadowMapDescriptorImageInfo;
	};
	TYPEDEF_SHARED_PTR_ARGS(ShadowMapRenderPipeline);
}