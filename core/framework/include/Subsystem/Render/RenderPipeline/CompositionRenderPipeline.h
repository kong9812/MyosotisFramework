// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class CompositionRenderPipeline : public RenderPipelineBase
	{
	public:
		CompositionRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors),
			m_shaderBase({}),
			m_lightingResultDescriptorImageInfo({}) {
		}
		~CompositionRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void CreateShaderObject();

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;

		ShaderBase m_shaderBase;
		VkDescriptorImageInfo m_lightingResultDescriptorImageInfo;
	};
	TYPEDEF_SHARED_PTR_ARGS(CompositionRenderPipeline);
}