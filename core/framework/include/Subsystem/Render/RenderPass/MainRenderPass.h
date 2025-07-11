// Copyright (c) 2025 kong9812
#include "RenderPassBase.h"
#include "RenderSwapchain.h"

namespace MyosotisFW::System::Render
{
	class MainRenderPass : public RenderPassBase
	{
	public:
		enum class SubPass : uint32_t
		{
			GBufferFill,
			Lighting,
			Composition,
			COUNT
		};

		enum class Attachments : uint32_t
		{
			MainRenderTarget,
			LightingResultImage,
			GBufferPosition,
			GBufferNormal,
			GBufferBaseColor,
			IdMap,
			DepthStencil,
			COUNT
		};

	public:
		MainRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const uint32_t& width, const uint32_t& height) :
			RenderPassBase(device, resources, width, height) {
		}
		~MainRenderPass();

		void Initialize() override;
		void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex) override;
		void EndRender(const VkCommandBuffer& commandBuffer) override;

	private:
		void createFrameBuffers() override;

	};
	TYPEDEF_SHARED_PTR_ARGS(MainRenderPass)
}