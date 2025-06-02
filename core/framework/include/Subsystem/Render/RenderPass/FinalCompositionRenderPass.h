// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPassBase.h"
#include "RenderSwapchain.h"

namespace MyosotisFW::System::Render
{
	class FinalCompositionRenderPass : public RenderPassBase
	{
	public:
		enum class SubPass : uint32_t
		{
			Composition,
			COUNT
		};

		enum class Attachments : uint32_t
		{
			SwapchainImages,
			MainRenderTarget,
			COUNT
		};

	public:
		FinalCompositionRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
			RenderPassBase(device, resources, swapchain->GetWidth(), swapchain->GetHeight()),
			m_swapchain(swapchain) {
		}
		~FinalCompositionRenderPass();

		void Initialize() override;
		void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex) override;
		void EndRender(const VkCommandBuffer& commandBuffer) override;

	private:
		void createFrameBuffers() override;

	protected:
		RenderSwapchain_ptr m_swapchain;

	};
	TYPEDEF_SHARED_PTR_ARGS(FinalCompositionRenderPass)
}