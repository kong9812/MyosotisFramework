// Copyright (c) 2025 kong9812
#pragma once
#include "classPointer.h"

#include "iglfw.h"
#include "vkStruct.h"

#include "renderDevice.h"
#include "renderSwapchain.h"
#include "renderResources.h"
#include "debugGUI.h"
#include "staticMesh.h"
#include "fpsCamera.h"

namespace MyosotisFW::System::Render
{
	class RenderSubsystem
	{
	public:
		RenderSubsystem(GLFWwindow& glfwWindow, VkInstance& instance, VkSurfaceKHR& surface);
		~RenderSubsystem();

		void ResetMousePos(glm::vec2 mousePos);

		void ResistObject(ObjectBase_ptr& object);

		void Update(Utility::Vulkan::Struct::UpdateData updateData);
		void Render();
		void Resize(VkSurfaceKHR& surface, uint32_t width, uint32_t height);

	private:
		struct {
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		}m_semaphores;

		VkInstance m_instance;

		RenderDevice_ptr m_device;
		RenderSwapchain_ptr m_swapchain;
		RenderResources_ptr m_resources;
		DebugGUI_ptr m_debugGUI;
		Camera::FPSCamera_ptr m_fpsCamera;

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

		std::vector<ObjectBase_ptr> m_objects;

		void prepareDepthStencil();
		void prepareRenderPass();
		void prepareFrameBuffers();
		void prepareCommandBuffers();
		void prepareFences();
		void buildCommandBuffer(uint32_t bufferIndex);
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderSubsystem)
}