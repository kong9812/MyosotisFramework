// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class InteriorObjectDeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		InteriorObjectDeferredRenderPipeline(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const VkRenderPass& renderPass);
		~InteriorObjectDeferredRenderPipeline();

		void CreateShaderObject(InteriorObjectShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(InteriorObjectDeferredRenderPipeline)
}