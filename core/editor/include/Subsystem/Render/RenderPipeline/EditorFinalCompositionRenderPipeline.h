// Copyright (c) 2025 kong9812
#pragma once
#include "FinalCompositionRenderPipeline.h"

namespace MyosotisFW::System::Render
{
	class EditorFinalCompositionRenderPipeline : public FinalCompositionRenderPipeline
	{
	public:
		EditorFinalCompositionRenderPipeline(const RenderDevice_ptr& device) :
			FinalCompositionRenderPipeline(device),
			m_editorRenderTargetDescriptorImageInfo{} {
		}
		~EditorFinalCompositionRenderPipeline() {};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass);
		void CreateShaderObject() override;
		void UpdateDescriptors();
		void Resize(const RenderResources_ptr& resources) override;

	private:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorImageInfo m_editorRenderTargetDescriptorImageInfo;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(EditorFinalCompositionRenderPipeline);
}