// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"

namespace MyosotisFW::System::Render
{
	class GizmoPipeline : public RenderPipelineBase
	{
	public:
		GizmoPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~GizmoPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void UpdateModel(const Transform& transform);

	private:
		struct PushConstant {
			glm::mat4 model;
		}pushConstant;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(GizmoPipeline);
}