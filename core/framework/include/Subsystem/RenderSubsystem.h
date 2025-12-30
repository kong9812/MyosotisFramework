// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include <vulkan/vulkan.h>
#include <vector>

#include "iglfw.h"
#include "Structs.h"
#include "AppInfo.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
	class MObjectRegistry;
	TYPEDEF_SHARED_PTR_FWD(MObjectRegistry);

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

		class SkyboxRenderPass;
		TYPEDEF_SHARED_PTR_FWD(SkyboxRenderPass);
		class VisibilityBufferPhase1RenderPass;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferPhase1RenderPass);
		class VisibilityBufferPhase2RenderPass;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferPhase2RenderPass);
		class LightingRenderPass;
		TYPEDEF_SHARED_PTR_FWD(LightingRenderPass);
		class LightmapBakingPass;
		TYPEDEF_SHARED_PTR_FWD(LightmapBakingPass);

		class SkyboxPipeline;
		TYPEDEF_SHARED_PTR_FWD(SkyboxPipeline);
		class VisibilityBufferPhase1Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferPhase1Pipeline);
		class VisibilityBufferPhase2Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferPhase2Pipeline);
		class LightingPipeline;
		TYPEDEF_SHARED_PTR_FWD(LightingPipeline);
		class LightmapBakingPipeline;
		TYPEDEF_SHARED_PTR_FWD(LightmapBakingPipeline);
		class RayTracingPipeline;
		TYPEDEF_SHARED_PTR_FWD(RayTracingPipeline);

		class HiZDepthComputePipeline;
		TYPEDEF_SHARED_PTR_FWD(HiZDepthComputePipeline);

		class AccelerationStructureManager;
		TYPEDEF_SHARED_PTR_FWD(AccelerationStructureManager);
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
			m_objectRegistry(nullptr),
			m_accelerationStructureManager(nullptr),
			m_frameCounter(0),
			m_vkCmdBeginDebugUtilsLabelEXT(nullptr),
			m_vkCmdEndDebugUtilsLabelEXT(nullptr),
			m_skyboxRenderPass(nullptr),
			m_visibilityBufferPhase1RenderPass(nullptr),
			m_visibilityBufferPhase2RenderPass(nullptr),
			m_lightingRenderPass(nullptr),
			m_lightmapBakingPass(nullptr),
			m_skyboxPipeline(nullptr),
			m_visibilityBufferPhase1Pipeline(nullptr),
			m_visibilityBufferPhase2Pipeline(nullptr),
			m_lightingPipeline(nullptr),
			m_lightmapBakingPipeline(nullptr),
			m_rayTracingPipeline(nullptr),
			m_hiZDepthComputePipeline(nullptr),
			m_fences(),
			m_vbDispatchInfoCount(0),
			m_semaphores(),
			m_stopRender(false) {
		}
		~RenderSubsystem();

		void ResetMousePos(const glm::vec2& mousePos);

		void RegisterObject(const MObject_ptr& object);
		RenderResources_ptr GetRenderResources() { return m_resources; }
		Camera::CameraBase_ptr GetMainCamera() { return m_mainCamera; }
		MObjectRegistry_ptr GetMObjectRegistry() { return m_objectRegistry; }

		virtual void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface);
		virtual void Update(const UpdateData& updateData);
		void Render();
		void ResetGameStage();
		void Resize(const VkSurfaceKHR& surface, const glm::ivec2& screenSize);

		std::vector<MObject_ptr> GetObjects() { return m_objects; }

	protected:
		void createHiZDepth(const VkCommandBuffer commandBuffer, const uint32_t dstFrameIndex, const uint32_t srcFrameIndex, const VkSemaphore wait, const VkSemaphore signal);
		void createVBufferPhase1(const uint32_t currentFrameIndex);
		void createVBufferPhase2(const uint32_t currentFrameIndex);
		void render(const uint32_t currentFrameIndex, const uint32_t currentSwapchainImageIndex);

	protected:
		void initializeRenderDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
		void initializeRenderSwapchain(const VkSurfaceKHR& surface);
		void initializeRenderDescriptors();
		virtual void initializeRenderResources();
		void initializeCommandPool();
		void initializeSemaphore();
		void initializeFence();
		void initializeDebugUtils(const VkInstance& instance);
		virtual void initializeRenderPass();
		virtual void initializeRenderPipeline();
		virtual void initializeComputePipeline();
		void initializeAccelerationStructureManager();
		virtual void resizeRenderPass();
		virtual void resizeRenderPipeline();

		void CopyMainRenderTargetToSwapchainImage(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t swapchainImageIndex);
		void CopyRayTracingRenderTargetToSwapchainImage(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t swapchainImageIndex);

	protected:
		struct {
			VkSemaphore completeHiZPhase1[AppInfo::g_maxInFlightFrameCount];
			VkSemaphore completeVBufferPhase1[AppInfo::g_maxInFlightFrameCount];
			VkSemaphore completeHiZPhase2[AppInfo::g_maxInFlightFrameCount];
			VkSemaphore completeVBufferPhase2[AppInfo::g_maxInFlightFrameCount];
			VkSemaphore completeRender[AppInfo::g_maxInFlightFrameCount];
			VkSemaphore imageAvailable[AppInfo::g_maxInFlightFrameCount];
		}m_semaphores;
		struct {
			VkFence inFlightFrameFence[AppInfo::g_maxInFlightFrameCount];
		}m_fences;
		struct {
			std::vector<VkCommandBuffer> completeHiZPhase1;
			std::vector<VkCommandBuffer> completeHiZPhase2;
			std::vector<VkCommandBuffer> createVBufferPhase1;
			std::vector<VkCommandBuffer> createVBufferPhase2;
			std::vector<VkCommandBuffer> render;
		}m_commandBuffers;

		bool m_stopRender;
		uint32_t m_frameCounter;
		uint32_t m_vbDispatchInfoCount;

		RenderDevice_ptr m_device;
		RenderSwapchain_ptr m_swapchain;
		RenderResources_ptr m_resources;
		MObjectRegistry_ptr m_objectRegistry;
		AccelerationStructureManager_ptr m_accelerationStructureManager;

		RenderDescriptors_ptr m_renderDescriptors;

		Camera::CameraBase_ptr m_mainCamera;

		std::vector<MObject_ptr> m_objects;

		PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
		PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;

	protected:
		SkyboxRenderPass_ptr m_skyboxRenderPass;
		VisibilityBufferPhase1RenderPass_ptr m_visibilityBufferPhase1RenderPass;
		VisibilityBufferPhase2RenderPass_ptr m_visibilityBufferPhase2RenderPass;
		LightingRenderPass_ptr m_lightingRenderPass;
		LightmapBakingPass_ptr m_lightmapBakingPass;

	protected:
		HiZDepthComputePipeline_ptr m_hiZDepthComputePipeline;
		SkyboxPipeline_ptr m_skyboxPipeline;
		VisibilityBufferPhase1Pipeline_ptr m_visibilityBufferPhase1Pipeline;
		VisibilityBufferPhase2Pipeline_ptr m_visibilityBufferPhase2Pipeline;
		LightingPipeline_ptr m_lightingPipeline;
		LightmapBakingPipeline_ptr m_lightmapBakingPipeline;
		RayTracingPipeline_ptr m_rayTracingPipeline;
	};
	TYPEDEF_SHARED_PTR(RenderSubsystem);
}