// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class DeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		DeferredRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass);
		~DeferredRenderPipeline();

		void CreateShaderObject(StaticMeshShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(DeferredRenderPipeline)
}