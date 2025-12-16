// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class VisibilityBufferRenderPhase1Pipeline : public RenderPipelineBase
	{
	public:
		VisibilityBufferRenderPhase1Pipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors) {
		}
		~VisibilityBufferRenderPhase1Pipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t vbDispatchInfoCount);

	private:
		struct {
			float hiZMipLevelMax;
			uint32_t hiZSamplerID;
			uint32_t vbDispatchInfoCount;
			//uint32_t checkFalseNegativeMesh;
		}pushConstant;

		uint32_t m_hiZSamplerID;
		uint32_t m_primaryDepthSamplerID;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(VisibilityBufferRenderPhase1Pipeline);
}