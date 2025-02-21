// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

#include "iglfw.h"
#include "Structs.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"
#include "RenderResources.h"
#include "DebugGUI.h"
#include "StaticMesh.h"
#include "FpsCamera.h"

#include "ShadowMapRenderPipeline.h"
#include "DeferredRenderPipeline.h"
#include "CompositionRenderPipeline.h"
#include "TransparentRenderPipeline.h"

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
		void MeshRender();
		void EndRender();
		void Resize(VkSurfaceKHR& surface, uint32_t width, uint32_t height);

		std::vector<ObjectBase_ptr> GetObjects() { return m_objects; }

	private:
		struct {
			VkSemaphore presentComplete;
			VkSemaphore computeComplete;
			VkSemaphore renderComplete;
		}m_semaphores;

		struct {
			RenderPass lighting;
			RenderPass staticMesh;
		}m_renderPass;

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

		std::vector<VkFence> m_fences;

		uint32_t m_currentBufferIndex;

		VkQueue m_graphicsQueue;
		VkQueue m_computeQueue;

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

		PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
		PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;

	private:
		VMAImage m_position;
		VMAImage m_normal;
		VMAImage m_baseColor;
		VMAImage m_shadowMap;
		ShaderBase m_compositionShaderBase;
		void prepareDeferredRendering();

		ShadowMapRenderPipeline_ptr m_shadowMapRenderPipeline;
		DeferredRenderPipeline_ptr m_deferredRenderPipeline;
		CompositionRenderPipeline_ptr m_compositionRenderPipeline;
		TransparentRenderPipeline_ptr m_transparentRenderPipeline;

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