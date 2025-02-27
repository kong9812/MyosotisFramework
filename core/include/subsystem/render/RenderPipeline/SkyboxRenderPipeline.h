// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class SkyboxRenderPipeline : public RenderPipelineBase
	{
	public:
		SkyboxRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass);
		~SkyboxRenderPipeline();

		void CreateShaderObject(SkyboxShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(SkyboxRenderPipeline)
}