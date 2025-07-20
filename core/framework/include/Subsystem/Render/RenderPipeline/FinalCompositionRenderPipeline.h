// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class FinalCompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		FinalCompositionRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors),
			m_descriptorSetLayout(VK_NULL_HANDLE),
			m_descriptorSet(VK_NULL_HANDLE),
			m_shaderBase({}),
			m_mainRenderTargetDescriptorImageInfo({}) {
		}
		~FinalCompositionRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		virtual void CreateShaderObject();

	protected:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;

		VkDescriptorImageInfo m_mainRenderTargetDescriptorImageInfo;
		ShaderBase m_shaderBase;
	};
	TYPEDEF_SHARED_PTR_ARGS(FinalCompositionRenderPipeline);
}