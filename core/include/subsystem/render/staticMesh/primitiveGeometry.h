// Copyright (c) 2025 kong9812
#pragma once
#include "staticMesh.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		~PrimitiveGeometry();

		void BindCommandBuffer(VkCommandBuffer commandBuffer) override;

	private:
		virtual void loadAssets() override;
		virtual void prepareUniformBuffers() override;
		virtual void prepareShaderStorageBuffers() override {};

		virtual void prepareDescriptors() override;
		virtual void prepareRenderPipeline() override;

	};
	TYPEDEF_SHARED_PTR_ARGS(PrimitiveGeometry)
}