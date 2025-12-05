// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPassBase.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderSwapchain;
	TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);

	class SkyboxRenderPass : public RenderPassBase
	{
	public:
		enum class SubPass : uint32_t
		{
			Skybox,
			COUNT
		};

		enum class Attachments : uint32_t
		{
			MainRenderTarget,
			COUNT
		};

	public:
		SkyboxRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain);
		~SkyboxRenderPass();

		void Initialize() override;
		void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t currentBufferIndex) override;
		void EndRender(const VkCommandBuffer& commandBuffer) override;

	private:
		void createFrameBuffers() override;
	};
	TYPEDEF_SHARED_PTR_ARGS(SkyboxRenderPass);
}