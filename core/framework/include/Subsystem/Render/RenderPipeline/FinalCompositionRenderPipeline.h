// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	class FinalCompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		FinalCompositionRenderPipeline(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const VkRenderPass& renderPass);
		~FinalCompositionRenderPipeline();

		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void CreateShaderObject(const VMAImage& mainRenderTarget);

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		ShaderBase m_shaderBase;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(FinalCompositionRenderPipeline)
}