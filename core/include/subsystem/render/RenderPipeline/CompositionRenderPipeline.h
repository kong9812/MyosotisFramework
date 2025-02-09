// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class CompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		CompositionRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass);
		~CompositionRenderPipeline();

		void CreateShaderObject(ShaderBase& shaderBase, VMAImage position, VMAImage baseColor);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(CompositionRenderPipeline)
}