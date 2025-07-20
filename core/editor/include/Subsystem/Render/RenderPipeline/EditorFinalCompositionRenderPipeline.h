// Copyright (c) 2025 kong9812
#pragma once
#include "FinalCompositionRenderPipeline.h"

namespace MyosotisFW::System::Render
{
	class EditorFinalCompositionRenderPipeline : public FinalCompositionRenderPipeline
	{
	public:
		EditorFinalCompositionRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			FinalCompositionRenderPipeline(device, descriptors),
			m_editorRenderTargetDescriptorImageInfo{} {
		}
		~EditorFinalCompositionRenderPipeline() {}

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass);
		void CreateShaderObject() override;

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorImageInfo m_editorRenderTargetDescriptorImageInfo;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(EditorFinalCompositionRenderPipeline);
}