// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class FinalCompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		FinalCompositionRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device),
			m_shaderBase({}),
			m_mainRenderTargetDescriptorImageInfo({}) {
		}
		~FinalCompositionRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		virtual void CreateShaderObject();
		void UpdateDescriptors();
		void Resize(const RenderResources_ptr& resources) override;

	protected:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorImageInfo m_mainRenderTargetDescriptorImageInfo;
		ShaderBase m_shaderBase;
	};
	TYPEDEF_SHARED_PTR_ARGS(FinalCompositionRenderPipeline);
}