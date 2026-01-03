// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"

namespace MyosotisFW::System::Render
{
	class SkyboxPipeline : public RenderPipelineBase
	{
	public:
		SkyboxPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~SkyboxPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct PushConstant {
			uint32_t skyboxTextureID;
		}pushConstant;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(SkyboxPipeline);
}