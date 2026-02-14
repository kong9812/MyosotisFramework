// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"
#include "RenderPipelineBase.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	class FogPipeline : public RenderPipelineBase
	{
	public:
		FogPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant(),
			m_active(true) {
		}
		~FogPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex);

	private:
		struct PushConstant {
			uint32_t depthTextureID;
			float startDist;
			float endDist;
			float _cp1;
			glm::vec4 fogColor;
		}pushConstant[AppInfo::g_maxInFlightFrameCount];
		bool m_active;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(FogPipeline);
}