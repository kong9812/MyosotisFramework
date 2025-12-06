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

		class SkyboxRenderPass;
		TYPEDEF_SHARED_PTR_FWD(SkyboxRenderPass);
		class TerrainRenderPass;
		TYPEDEF_SHARED_PTR_FWD(TerrainRenderPass);
		class VisibilityBufferRenderPass;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferRenderPass);
		class LightingRenderPass;
		TYPEDEF_SHARED_PTR_FWD(LightingRenderPass);

		class SkyboxPipeline;
		TYPEDEF_SHARED_PTR_FWD(SkyboxPipeline);
		class TerrainPipeline;
		TYPEDEF_SHARED_PTR_FWD(TerrainPipeline);
		class VisibilityBufferRenderPhase1Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferRenderPhase1Pipeline);
		class VisibilityBufferRenderPhase2Pipeline;
		TYPEDEF_SHARED_PTR_FWD(VisibilityBufferRenderPhase2Pipeline);
		class LightingPipeline;
		TYPEDEF_SHARED_PTR_FWD(LightingPipeline);

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
			m_skyboxRenderPass(nullptr),
			m_terrainRenderPass(nullptr),
			m_visibilityBufferRenderPass(nullptr),
			m_lightingRenderPass(nullptr),
			m_skyboxPipeline(nullptr),
			m_terrainPipeline(nullptr),
			m_visibilityBufferRenderPhase1Pipeline(nullptr),
			m_visibilityBufferRenderPhase2Pipeline(nullptr),
			m_lightingPipeline(nullptr),
			m_hiZDepthComputePipeline(nullptr),
			m_renderFence(VK_NULL_HANDLE),
			m_vbDispatchInfoCount(0) {
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
		MObjectRegistry_ptr GetMObjectRegistry() { return m_objectRegistry; }

		virtual void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface);
		virtual void Update(const UpdateData& updateData);
		void BeginCompute();
		void BeginRender();
		void SkyboxRender();
		void TerrainRender();
		void MeshShaderRender();
		void LightingRender();
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

		void CopyMainRenderTargetToSwapchainImage();

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
		MObjectRegistry_ptr m_objectRegistry;

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
		SkyboxRenderPass_ptr m_skyboxRenderPass;
		TerrainRenderPass_ptr m_terrainRenderPass;
		VisibilityBufferRenderPass_ptr m_visibilityBufferRenderPass;
		LightingRenderPass_ptr m_lightingRenderPass;

	protected:
		SkyboxPipeline_ptr m_skyboxPipeline;
		TerrainPipeline_ptr m_terrainPipeline;
		VisibilityBufferRenderPhase1Pipeline_ptr m_visibilityBufferRenderPhase1Pipeline;
		VisibilityBufferRenderPhase2Pipeline_ptr m_visibilityBufferRenderPhase2Pipeline;
		LightingPipeline_ptr m_lightingPipeline;

	protected:
		HiZDepthComputePipeline_ptr m_hiZDepthComputePipeline;

	protected:
		uint32_t m_vbDispatchInfoCount;
	};
	TYPEDEF_SHARED_PTR(RenderSubsystem);
}