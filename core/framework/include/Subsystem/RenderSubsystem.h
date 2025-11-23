// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

#include "iglfw.h"
#include "Structs.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);

	namespace System::Render
	{
		class RenderDevice;
		TYPEDEF_SHARED_PTR_FWD(RenderDevice);
		class RenderSwapchain;
		TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);
		class RenderResources;
		TYPEDEF_SHARED_PTR_FWD(RenderResources);

		class DescriptorPool;
		TYPEDEF_SHARED_PTR_FWD(DescriptorPool);

		class SceneInfoDescriptorSet;
		TYPEDEF_SHARED_PTR_FWD(SceneInfoDescriptorSet);
		class ObjectInfoDescriptorSet;
		TYPEDEF_SHARED_PTR_FWD(ObjectInfoDescriptorSet);
		class MeshInfoDescriptorSet;
		TYPEDEF_SHARED_PTR_FWD(MeshInfoDescriptorSet);
		class TextureDescriptorSet;
		TYPEDEF_SHARED_PTR_FWD(TextureDescriptorSet);

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
		class MeshShaderRenderPass;
		TYPEDEF_SHARED_PTR_FWD(MeshShaderRenderPass);

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
		class VisibilityBufferRenderPhase1Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferRenderPhase1Pipeline);
		class VisibilityBufferRenderPhase2Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferRenderPhase2Pipeline);

		class HiZDepthComputePipeline;
		TYPEDEF_SHARED_PTR_FWD(HiZDepthComputePipeline);
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
			m_mainCamera(nullptr),
			m_submitPipelineStages(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT),
			m_renderCommandPool(VK_NULL_HANDLE),
			m_computeCommandPool(VK_NULL_HANDLE),
			m_transferCommandPool(VK_NULL_HANDLE),
			m_currentBufferIndex(0),
			m_vkCmdBeginDebugUtilsLabelEXT(nullptr),
			m_vkCmdEndDebugUtilsLabelEXT(nullptr),
			m_shadowMapRenderPass(nullptr),
			m_mainRenderPass(nullptr),
			m_finalCompositionRenderPass(nullptr),
			m_meshShaderRenderPass(nullptr),
			m_skyboxRenderPipeline(nullptr),
			m_shadowMapRenderPipeline(nullptr),
			m_deferredRenderPipeline(nullptr),
			m_lightingRenderPipeline(nullptr),
			m_compositionRenderPipeline(nullptr),
			m_finalCompositionRenderPipeline(nullptr),
			m_interiorObjectDeferredRenderPipeline(nullptr),
			m_visibilityBufferRenderPhase1Pipeline(nullptr),
			m_visibilityBufferRenderPhase2Pipeline(nullptr),
			m_hiZDepthComputePipeline(nullptr),
			m_renderFence(VK_NULL_HANDLE) {
			m_semaphores.presentComplete = VK_NULL_HANDLE;
			m_semaphores.computeComplete = VK_NULL_HANDLE;
			m_semaphores.renderComplete = VK_NULL_HANDLE;
			m_submitInfo = {};
			m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		}
		~RenderSubsystem();

		void ResetMousePos(const glm::vec2& mousePos);

		void RegisterObject(const MObject_ptr& object);
		RenderResources_ptr GetRenderResources() { return m_resources; }
		Camera::CameraBase_ptr GetMainCamera() { return m_mainCamera; }

		virtual void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface);
		virtual void Update(const UpdateData& updateData);
		void BeginCompute();
		void BeginRender();
		void MeshShaderRender();
		void EndRender();
		void ResetGameStage();
		void Resize(const VkSurfaceKHR& surface, const uint32_t width, const uint32_t height);

		std::vector<MObject_ptr> GetObjects() { return m_objects; }

	protected:
		void initializeRenderDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
		void initializeRenderSwapchain(const VkSurfaceKHR& surface);
		void initializeRenderDescriptors();
		virtual void initializeRenderResources();
		void initializeCommandPool();
		void initializeSemaphore();
		void initializeFence();
		void initializeSubmitInfo();
		void initializeDebugUtils(const VkInstance& instance);
		virtual void initializeRenderPass();
		virtual void initializeRenderPipeline();
		virtual void initializeComputePipeline();
		virtual void resizeRenderPass(const uint32_t width, const uint32_t height);

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

		DescriptorPool_ptr m_descriptorPool;
		SceneInfoDescriptorSet_ptr m_sceneInfoDescriptorSet;
		ObjectInfoDescriptorSet_ptr m_objectInfoDescriptorSet;
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;
		TextureDescriptorSet_ptr m_textureDescriptorSet;

		Camera::CameraBase_ptr m_mainCamera;

		VkSubmitInfo m_submitInfo;
		VkPipelineStageFlags m_submitPipelineStages;

		VkCommandPool m_renderCommandPool;
		VkCommandPool m_computeCommandPool;
		VkCommandPool m_transferCommandPool;
		std::vector<VkCommandBuffer> m_renderCommandBuffers;
		std::vector<VkCommandBuffer> m_computeCommandBuffers;

		uint32_t m_currentBufferIndex;

		std::vector<MObject_ptr> m_objects;

		PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
		PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;

	protected:
		ShadowMapRenderPass_ptr m_shadowMapRenderPass;
		MainRenderPass_ptr m_mainRenderPass;
		FinalCompositionRenderPass_ptr m_finalCompositionRenderPass;
		MeshShaderRenderPass_ptr m_meshShaderRenderPass;

	protected:
		SkyboxRenderPipeline_ptr m_skyboxRenderPipeline;
		ShadowMapRenderPipeline_ptr m_shadowMapRenderPipeline;
		DeferredRenderPipeline_ptr m_deferredRenderPipeline;
		LightingRenderPipeline_ptr m_lightingRenderPipeline;
		CompositionRenderPipeline_ptr m_compositionRenderPipeline;
		FinalCompositionRenderPipeline_ptr m_finalCompositionRenderPipeline;
		InteriorObjectDeferredRenderPipeline_ptr m_interiorObjectDeferredRenderPipeline;
		VisibilityBufferRenderPhase1Pipeline_ptr m_visibilityBufferRenderPhase1Pipeline;
		VisibilityBufferRenderPhase2Pipeline_ptr m_visibilityBufferRenderPhase2Pipeline;

	protected:
		HiZDepthComputePipeline_ptr m_hiZDepthComputePipeline;
	};
	TYPEDEF_SHARED_PTR(RenderSubsystem);
}