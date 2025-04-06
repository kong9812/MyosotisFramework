// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPassBase.h"

namespace MyosotisFW::System::Render
{
	class ShadowMapRenderPass : public RenderPassBase
	{
	public:
		ShadowMapRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const uint32_t& width, const uint32_t& height) :
			RenderPassBase(device, resources, width, height) {
		}
		~ShadowMapRenderPass();

		void Initialize() override;
		void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex) override;
		void EndRender(const VkCommandBuffer& commandBuffer) override;

	};
	TYPEDEF_SHARED_PTR_ARGS(ShadowMapRenderPass)
}