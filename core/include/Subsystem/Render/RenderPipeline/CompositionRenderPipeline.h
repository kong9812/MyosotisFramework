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
		CompositionRenderPipeline(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const VkRenderPass& renderPass);
		~CompositionRenderPipeline();

		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void UpdateDirectionalLightInfo(const DirectionalLightInfo& lightInfo);
		void UpdateCameraPosition(const glm::vec4& position);
		void CreateShaderObject(const VMAImage& position, const VMAImage& normal, const VMAImage& baseColor, const VkDescriptorImageInfo& shadowMapImageInfo);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		CompositionShaderObject m_compositionShaderObject;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(CompositionRenderPipeline)
}