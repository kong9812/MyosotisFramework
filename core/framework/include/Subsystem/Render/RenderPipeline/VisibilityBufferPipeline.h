// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class VisibilityBufferPipeline : public RenderPipelineBase
	{
	public:
		VisibilityBufferPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors) {
		}
		~VisibilityBufferPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t vbDispatchInfoCount);

	private:
		struct PushConstant {
			float hiZMipLevelMax;
			uint32_t hiZSamplerID;
			uint32_t vbDispatchInfoCount;
		}pushConstant[AppInfo::g_maxInFlightFrameCount];

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(VisibilityBufferPipeline);
}