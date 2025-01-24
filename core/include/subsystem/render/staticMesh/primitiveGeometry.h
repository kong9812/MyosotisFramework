// Copyright (c) 2025 kong9812
#pragma once
#include "staticMesh.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry() : StaticMesh() {};
		~PrimitiveGeometry() {};

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		void Update(const Camera::CameraBase& camera) override;
		void BindCommandBuffer(VkCommandBuffer commandBuffer) override;
		void BindDebugGUIElement() override {};

	private:
		void loadAssets() override;
		void prepareUniformBuffers() override;
		void prepareShaderStorageBuffers() override {};

		void prepareDescriptors() override;
		void prepareRenderPipeline() override;

	};
	TYPEDEF_SHARED_PTR(PrimitiveGeometry)
}