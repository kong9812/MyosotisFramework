// Copyright (c) 2025 kong9812
#pragma once
#include <memory>
#include "render/renderDevice.h"
#include "render/renderSwapchain.h"
#include "libs/glfw.h"
#include "vkStruct.h"

namespace MyosotisFW::System::Render
{
	class RenderSubsystem
	{
	public:
		RenderSubsystem(VkInstance& instance, VkSurfaceKHR& surface);
		~RenderSubsystem();

		void Render();
		void Resize(VkSurfaceKHR& surface, uint32_t width, uint32_t height);

	private:
		struct {
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		}m_semaphores;

		VkInstance m_instance;

		RenderDevice_prt m_device;
		RenderSwapchain_prt m_swapchain;

		VkSubmitInfo m_submitInfo;
		VkPipelineStageFlags m_submitPipelineStages;

		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkFramebuffer> m_frameBuffers;

		std::vector<VkFence> m_fences;

		uint32_t m_currentBufferIndex;

		VkPipelineCache m_pipelineCache;

		VkQueue m_queue;
		
		VkRenderPass m_renderPass;
		Utility::Vulkan::Struct::DeviceImage m_depthStencil;

		void prepareDepthStencil();
		void prepareRenderPass();
		void prepareFrameBuffers();
		void prepareCommandBuffers();
		void prepareFences();
		void buildCommandBuffers();
	};

	using RenderSubsystem_prt = std::unique_ptr<RenderSubsystem>;
}