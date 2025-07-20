// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

#include "iglfw.h"
#include "Structs.h"

// 前方宣言
namespace MyosotisFW
{
	class StageObject;
	TYPEDEF_SHARED_PTR_FWD(StageObject);

	namespace System::Render
	{
		class RenderDevice;
		TYPEDEF_SHARED_PTR_FWD(RenderDevice);
		class RenderSwapchain;
		TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);
		class RenderResources;
		TYPEDEF_SHARED_PTR_FWD(RenderResources);
		class RenderDescriptors;
		TYPEDEF_SHARED_PTR_FWD(RenderDescriptors);

		class StaticMesh;
		TYPEDEF_SHARED_PTR_FWD(StaticMesh);
		class DebugGUI;
		TYPEDEF_SHARED_PTR_FWD(DebugGUI);
		namespace Camera
		{
			class CameraBase;
			TYPEDEF_SHARED_PTR_FWD(CameraBase);
		}

		class ShadowMapRenderPass;
		TYPEDEF_SHARED_PTR_FWD(ShadowMapRenderPass);
		class MainRenderPass;
		TYPEDEF_SHARED_PTR_FWD(MainRenderPass);
		class FinalCompositionRenderPass;
		TYPEDEF_SHARED_PTR_FWD(FinalCompositionRenderPass);
		class BindlessResourcesRenderPass;
		TYPEDEF_SHARED_PTR_FWD(BindlessResourcesRenderPass);

		class SkyboxRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(SkyboxRenderPipeline);
		class ShadowMapRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(ShadowMapRenderPipeline);
		class DeferredRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(DeferredRenderPipeline);
		class LightingRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(LightingRenderPipeline);
		class CompositionRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(CompositionRenderPipeline);
		class FinalCompositionRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(FinalCompositionRenderPipeline);
		class InteriorObjectDeferredRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(InteriorObjectDeferredRenderPipeline);
		class BindlessResourcesRenderPipeline;
		TYPEDEF_SHARED_PTR_FWD(BindlessResourcesRenderPipeline);
	}
}

namespace MyosotisFW::System::Render
{
	class RenderSubsystem
	{
	public:
		RenderSubsystem() :
			m_device(nullptr),
			m_swapchain(nullptr),
			m_resources(nullptr),
			m_descriptors(nullptr),
			m_mainCamera(nullptr),
			m_submitPipelineStages(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT),
			m_renderCommandPool(VK_NULL_HANDLE),
			m_computeCommandPool(VK_NULL_HANDLE),
			m_transferCommandPool(VK_NULL_HANDLE),
			m_currentBufferIndex(0),
			m_descriptorPool(VK_NULL_HANDLE),
			m_vkCmdBeginDebugUtilsLabelEXT(nullptr),
			m_vkCmdEndDebugUtilsLabelEXT(nullptr),
			m_shadowMapRenderPass(nullptr),
			m_mainRenderPass(nullptr),
			m_finalCompositionRenderPass(nullptr),
			m_bindlessResourcesRenderPass(nullptr),
			m_skyboxRenderPipeline(nullptr),
			m_shadowMapRenderPipeline(nullptr),
			m_deferredRenderPipeline(nullptr),
			m_lightingRenderPipeline(nullptr),
			m_compositionRenderPipeline(nullptr),
			m_finalCompositionRenderPipeline(nullptr),
			m_interiorObjectDeferredRenderPipeline(nullptr),
			m_bindlessResourcesRenderPipeline(nullptr),
			m_renderFence(VK_NULL_HANDLE) {
			m_semaphores.presentComplete = VK_NULL_HANDLE;
			m_semaphores.computeComplete = VK_NULL_HANDLE;
			m_semaphores.renderComplete = VK_NULL_HANDLE;
			m_submitInfo = {};
			m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		}
		~RenderSubsystem();

		void ResetMousePos(const glm::vec2& mousePos);

		void RegisterObject(const StageObject_ptr& object);
		RenderResources_ptr GetRenderResources() { return m_resources; }
		Camera::CameraBase_ptr GetMainCamera() { return m_mainCamera; }

		virtual void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface);
		virtual void Update(const UpdateData& updateData);
		void FrustumCuller();
		void BeginRender();
		void ShadowRender();
		void MainRender();
		void FinalCompositionRender();
		void BindlessResourcesRender();
		void EndRender();
		void ResetGameStage();
		void Resize(const VkSurfaceKHR& surface, const uint32_t& width, const uint32_t& height);

		std::vector<StageObject_ptr> GetObjects() { return m_objects; }

	protected:
		void initializeRenderDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
		void initializeRenderSwapchain(const VkSurfaceKHR& surface);
		void initializeRenderDescriptors();
		virtual void initializeRenderResources();
		void initializeCommandPool();
		void initializeFrustumCuller();
		void initializeSemaphore();
		void initializeFence();
		void initializeSubmitInfo();
		void initializeDebugUtils(const VkInstance& instance);
		virtual void initializeRenderPass();
		virtual void initializeRenderPipeline();
		virtual void resizeRenderPass(const uint32_t& width, const uint32_t& height);

	protected:
		struct {
			VkSemaphore presentComplete;
			VkSemaphore computeComplete;
			VkSemaphore renderComplete;
		}m_semaphores;

		VkFence m_renderFence;

		RenderDevice_ptr m_device;
		RenderSwapchain_ptr m_swapchain;
		RenderResources_ptr m_resources;
		RenderDescriptors_ptr m_descriptors;

		Camera::CameraBase_ptr m_mainCamera;

		VkSubmitInfo m_submitInfo;
		VkPipelineStageFlags m_submitPipelineStages;

		VkCommandPool m_renderCommandPool;
		VkCommandPool m_computeCommandPool;
		VkCommandPool m_transferCommandPool;
		std::vector<VkCommandBuffer> m_renderCommandBuffers;
		std::vector<VkCommandBuffer> m_computeCommandBuffers;

		uint32_t m_currentBufferIndex;

		VkDescriptorPool m_descriptorPool;
		FrustumCullersShaderObject m_frustumCullerShaderObject;

		std::vector<StageObject_ptr> m_objects;

		PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
		PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;

	protected:
		ShadowMapRenderPass_ptr m_shadowMapRenderPass;
		MainRenderPass_ptr m_mainRenderPass;
		FinalCompositionRenderPass_ptr m_finalCompositionRenderPass;
		BindlessResourcesRenderPass_ptr m_bindlessResourcesRenderPass;

	protected:
		SkyboxRenderPipeline_ptr m_skyboxRenderPipeline;
		ShadowMapRenderPipeline_ptr m_shadowMapRenderPipeline;
		DeferredRenderPipeline_ptr m_deferredRenderPipeline;
		LightingRenderPipeline_ptr m_lightingRenderPipeline;
		CompositionRenderPipeline_ptr m_compositionRenderPipeline;
		FinalCompositionRenderPipeline_ptr m_finalCompositionRenderPipeline;
		InteriorObjectDeferredRenderPipeline_ptr m_interiorObjectDeferredRenderPipeline;
		BindlessResourcesRenderPipeline_ptr m_bindlessResourcesRenderPipeline;
	};
	TYPEDEF_SHARED_PTR(RenderSubsystem);
}