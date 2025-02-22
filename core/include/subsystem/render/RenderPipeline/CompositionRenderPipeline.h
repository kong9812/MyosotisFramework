// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	class CompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		CompositionRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass);
		~CompositionRenderPipeline();

		void BindCommandBuffer(VkCommandBuffer commandBuffer);
		void UpdateDirectionalLightInfo(DirectionalLightInfo lightInfo);
		void UpdateCameraPosition(glm::vec4 position);
		void CreateShaderObject(VMAImage position, VMAImage normal, VMAImage baseColor, VkDescriptorImageInfo shadowMapImageInfo);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) override;

		CompositionShaderObject m_compositionShaderObject;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(CompositionRenderPipeline)
}