// Copyright (c) 2025 kong9812
#pragma once
#include "classPointer.h"

#include "iglfw.h"
#include "Structs.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"
#include "RenderResources.h"
#include "DebugGUI.h"
#include "StaticMesh.h"
#include "FpsCamera.h"

namespace MyosotisFW::System::Render
{
	class RenderSubsystem
	{
	public:
		RenderSubsystem(GLFWwindow& glfwWindow, VkInstance& instance, VkSurfaceKHR& surface);
		~RenderSubsystem();

		void ResetMousePos(glm::vec2 mousePos);

		void RegisterObject(ObjectBase_ptr& object);
		RenderResources_ptr GetRenderResources() { return m_resources; }
		Camera::CameraBase_ptr GetMainCamera() { return m_mainCamera; }

		void Update(UpdateData updateData);
		void Compute();
		void BeginRender();
		void TransparentRender();
		void EndRender();
		void Resize(VkSurfaceKHR& surface, uint32_t width, uint32_t height);

		std::vector<ObjectBase_ptr> GetObjects() { return m_objects; }

	private:
		struct {
			VkSemaphore presentComplete;
			VkSemaphore computeComplete;
			VkSemaphore renderComplete;
		}m_semaphores;

		VkInstance m_instance;

		RenderDevice_ptr m_device;
		RenderSwapchain_ptr m_swapchain;
		RenderResources_ptr m_resources;
		Camera::CameraBase_ptr m_mainCamera;

		VkSubmitInfo m_submitInfo;
		VkPipelineStageFlags m_submitPipelineStages;

		VkCommandPool m_renderCommandPool;
		VkCommandPool m_computeCommandPool;
		std::vector<VkCommandBuffer> m_renderCommandBuffers;
		std::vector<VkCommandBuffer> m_computeCommandBuffers;
		std::vector<VkFramebuffer> m_frameBuffers;

		std::vector<VkFence> m_fences;

		uint32_t m_currentBufferIndex;

		VkQueue m_graphicsQueue;
		VkQueue m_computeQueue;

		VkRenderPass m_renderPass;
		DeviceImage m_depthStencil;

		VkDescriptorPool m_descriptorPool;
		FrustumCullersShaderObject m_frustumCullerShaderObject;

		std::vector<ObjectBase_ptr> m_objects;

		void prepareDepthStencil();
		void prepareRenderPass();
		void prepareFrameBuffers();
		void prepareCommandBuffers();
		void prepareFences();

		void prepareFrustumCuller();

	// callback
	private:
		using OnPressedSaveGameStageCallback = std::function<void()>;
		OnPressedSaveGameStageCallback m_onPressedSaveGameStageCallback;
		using OnPressedLoadGameStageCallback = std::function<void()>;
		OnPressedLoadGameStageCallback m_onPressedLoadGameStageCallback;
		using OnPressedCreateObjectCallback = std::function<void(ObjectType, glm::vec3)>;
		OnPressedCreateObjectCallback m_onPressedCreateObjectCallback;
;	public:
	void SetOnPressedSaveGameStageCallback(OnPressedSaveGameStageCallback callback);
	void SetOnPressedLoadGameStageCallback(OnPressedLoadGameStageCallback callback);
	void SetOnPressedCreateObjectCallback(OnPressedCreateObjectCallback callback);

	};
	TYPEDEF_SHARED_PTR_ARGS(RenderSubsystem)
}