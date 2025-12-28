// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class VisibilityBufferPhase1Pipeline : public RenderPipelineBase
	{
	public:
		VisibilityBufferPhase1Pipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors) {
		}
		~VisibilityBufferPhase1Pipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t vbDispatchInfoCount);
		void Resize(const RenderResources_ptr& resources) override;

	private:
		struct PushConstant {
			float hiZMipLevelMax;
			uint32_t hiZSamplerID;
			uint32_t pcVBDispatchInfoCount;
			uint32_t phase;
		}pushConstant[AppInfo::g_maxInFlightFrameCount];

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(VisibilityBufferPhase1Pipeline);
}