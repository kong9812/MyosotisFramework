// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "FinalCompositionRenderPass.h"
#include "EditorRenderResources.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderSwapchain;
	TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);

	class EditorFinalCompositionRenderPass : public FinalCompositionRenderPass
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
			EditorRenderTarget,
			COUNT
		};

	public:
		EditorFinalCompositionRenderPass(const RenderDevice_ptr& device, const EditorRenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
			FinalCompositionRenderPass(device, resources, swapchain) {
		};

		void Initialize() override;
		void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex) override;

	private:
		void createFrameBuffers() override;

	};
	TYPEDEF_SHARED_PTR_ARGS(EditorFinalCompositionRenderPass);
}