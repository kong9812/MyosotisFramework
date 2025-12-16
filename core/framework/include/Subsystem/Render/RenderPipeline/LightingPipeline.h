// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class LightingPipeline : public RenderPipelineBase
	{
	public:
		LightingPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~LightingPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct {
			uint32_t visibilityBufferTextureID;
		}pushConstant;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(LightingPipeline);
}