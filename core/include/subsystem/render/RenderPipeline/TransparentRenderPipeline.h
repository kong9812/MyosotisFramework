// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class TransparentRenderPipeline : public RenderPipelineBase
	{
	public:
		TransparentRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass);
		~TransparentRenderPipeline();

		void CreateShaderObject(StaticMeshShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(TransparentRenderPipeline)
}