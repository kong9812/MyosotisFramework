// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class DeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		DeferredRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device) {
		}
		~DeferredRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(StaticMeshShaderObject& shaderObject);
		void UpdateDescriptors(StaticMeshShaderObject& shaderObject);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(DeferredRenderPipeline);
}