// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class SkyboxRenderPipeline : public RenderPipelineBase
	{
	public:
		SkyboxRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device) {
		}
		~SkyboxRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(SkyboxShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(SkyboxRenderPipeline)
}