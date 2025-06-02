// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class InteriorObjectDeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		InteriorObjectDeferredRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device) {
		}
		~InteriorObjectDeferredRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(InteriorObjectShaderObject& shaderObject);
		void UpdateDescriptors(InteriorObjectShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(InteriorObjectDeferredRenderPipeline)
}