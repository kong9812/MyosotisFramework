// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <algorithm>
#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "MObject.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"
#include "RenderResources.h"
#include "MObjectRegistry.h"
#include "AccelerationStructureManager.h"

#include "RenderDescriptors.h"

#include "StaticMesh.h"
#include "FpsCamera.h"

#include "SkyboxRenderPass.h"
#include "VisibilityBufferPhase1RenderPass.h"
#include "VisibilityBufferPhase2RenderPass.h"
#include "LightingRenderPass.h"
#include "LightmapBakingPass.h"
#include "PostProcessRenderPass.h"

#include "SkyboxPipeline.h"
#include "VisibilityBufferPhase1Pipeline.h"
#include "VisibilityBufferPhase2Pipeline.h"
#include "LightingPipeline.h"
#include "LightmapBakingPipeline.h"
#include "RayTracingPipeline.h"

#include "HiZDepthComputePipeline.h"

#include "RenderQueue.h"
#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

#include "PrimitiveGeometry.h"

namespace {
	typedef struct
	{
		float distance;
		MyosotisFW::System::Render::StaticMesh_ptr staticMesh;
	}TransparentStaticMesh;	// Zソート用

	float g_debugTimer = 0.0f;
	float g_debugSpeed = 50.0f;
}

namespace MyosotisFW::System::Render
{
	RenderSubsystem::~RenderSubsystem()
	{
		m_device->GetGraphicsQueue()->WaitIdle();
		m_device->GetComputeQueue()->WaitIdle();
		m_device->GetTransferQueue()->WaitIdle();

		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			vkDestroyFence(*m_device, m_fences.inFlightFrameFence[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeHiZPhase1[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeVBufferPhase1[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeHiZPhase2[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeVBufferPhase2[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeRender[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.imageAvailable[i], m_device->GetAllocationCallbacks());
		}

		m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.createVBufferPhase1);
		m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.createVBufferPhase2);
		m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.render);
		m_device->GetGraphicsQueue()->DestroyCommandPool(*m_device, m_device->GetAllocationCallbacks());
		m_device->GetComputeQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.completeHiZPhase1);
		m_device->GetComputeQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.completeHiZPhase2);
		m_device->GetComputeQueue()->DestroyCommandPool(*m_device, m_device->GetAllocationCallbacks());
		m_device->GetTransferQueue()->DestroyCommandPool(*m_device, m_device->GetAllocationCallbacks());
	}

	void RenderSubsystem::ResetMousePos(const glm::vec2& mousePos)
	{
		if (!m_mainCamera) return;

		Object_Cast<Camera::FPSCamera>(m_mainCamera)->ResetMousePos(mousePos);
	}

	void RenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		// Vulkan Instance
		initializeRenderDevice(instance, surface);

		// Swapchain
		initializeRenderSwapchain(surface);

		// Command pool
		initializeCommandPool();

		// Descriptors
		initializeRenderDescriptors();

		// Resources
		initializeRenderResources();

		// ObjectRegistry
		initializeObjectRegistry();

		// Semaphore
		initializeSemaphore();

		// Fence
		initializeFence();

		// Acceleration Structure Manager
		initializeAccelerationStructureManager();

		// Debug Utils
		initializeDebugUtils(instance);

		// Render Pass
		initializeRenderPass();

		// Render Pipelines
		initializeRenderPipeline();

		// Compute Pipelines
		initializeComputePipeline();
	}

	void RenderSubsystem::Update(const UpdateData& updateData)
	{
		if (m_mainCamera)
		{
			m_mainCamera->Update(updateData);
		}
		else
		{
			for (const MObject_ptr& object : *m_objects)
			{
				if (object->IsCamera())
				{
					ComponentBaseHandle* handle = object->FindComponent(ComponentType::FPSCamera, true);
					if (handle)
					{
						m_mainCamera = Camera::Object_CastToCameraBase(handle->lock());
						m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
						m_mainCamera->SetMainCamera(true);
						break;
					}
				}
			}
		}

		const bool meshChanged = m_objectRegistry->IsMeshChanged();
		const bool transformChanged = m_objectRegistry->IsTransformChanged();

		if (meshChanged || transformChanged)
		{
			// Reset VBDispatchInfo
			m_renderDescriptors->GetObjectInfoDescriptorSet()->ResetVBDispatchInfo();

			// Reset mesh count
			m_vbDispatchInfoCount = 0;

			for (const MObject_ptr& object : *m_objects)
			{
				object->Update(updateData, m_mainCamera);

				std::vector<VBDispatchInfo> vbDispatchInfo = object->GetVBDispatchInfo();
				m_renderDescriptors->GetObjectInfoDescriptorSet()->AddObjectInfo(object->GetObjectInfo(), vbDispatchInfo);
				m_vbDispatchInfoCount += static_cast<uint32_t>(vbDispatchInfo.size());
			}
			m_renderDescriptors->GetObjectInfoDescriptorSet()->Update();
			if (meshChanged)
			{
				m_renderDescriptors->GetMeshInfoDescriptorSet()->Update();
			}
			m_accelerationStructureManager->RebuildTLAS();
			m_objectRegistry->ResetChangeFlags();
		}
		m_renderDescriptors->GetSceneInfoDescriptorSet()->Update();
		m_renderDescriptors->GetTextureDescriptorSet()->Update();

		auto key = updateData.keyActions.find(GLFW_KEY_X);
		if (key != updateData.keyActions.end())
		{
			if (key->second == GLFW_RELEASE)
			{
				m_lightmapBakingPipeline->Bake();
			}
		}

		m_accelerationStructureManager->Process();
		m_renderDescriptors->GetRayTracingDescriptorSet()->Update();
		m_renderDescriptors->GetMaterialDescriptorSet()->Update();
	}

	void RenderSubsystem::Render()
	{
		if (m_stopRender) return;

		const uint32_t currentFrameIndex = m_frameCounter % AppInfo::g_maxInFlightFrameCount;
		const uint32_t previousFrameIndex = (m_frameCounter + 1) % AppInfo::g_maxInFlightFrameCount;

		// これから使うFrameIndexのRenderSubmitが終わるまで待つ
		VK_VALIDATION(vkWaitForFences(*m_device, 1, &m_fences.inFlightFrameFence[currentFrameIndex], VK_TRUE, UINT64_MAX));

		// これから書き込むSwapchain ImageのIndexを取得
		uint32_t currentSwapchainImageIndex = 0;
		m_swapchain->AcquireNextImage(m_semaphores.imageAvailable[currentFrameIndex], currentSwapchainImageIndex);

		// 実行準備 (inFlightFrameFenceのリセット)
		VK_VALIDATION(vkResetFences(*m_device, 1, &m_fences.inFlightFrameFence[currentFrameIndex]));
		// コマンドバッファ取り出す

		// Compute (Hi-Z Depth)
		// 前FrameのVBufferPhase2が終わったら、このFrameのHi-ZDepthが作れる
		createHiZDepth(m_commandBuffers.completeHiZPhase1[currentFrameIndex], currentFrameIndex, previousFrameIndex, m_semaphores.completeVBufferPhase2[previousFrameIndex], m_semaphores.completeHiZPhase1[currentFrameIndex]);

		// Graphics VBufferPhase1 (VisibilityBuffer)
		createVBufferPhase1(currentFrameIndex);

		// Compute (Hi-Z Depth)
		// このFrameのVBufferPhase1が終わったら、Phase1のHi-ZDepthが作れる
		createHiZDepth(m_commandBuffers.completeHiZPhase2[currentFrameIndex], currentFrameIndex, currentFrameIndex, m_semaphores.completeVBufferPhase1[currentFrameIndex], m_semaphores.completeHiZPhase2[currentFrameIndex]);

		// Graphics VBufferPhase2 (VisibilityBuffer)
		createVBufferPhase2(currentFrameIndex);

		// Graphics Render (Skybox,Lighting,LightMap,RayTracing...)
		beginRender(currentFrameIndex);
		render(currentFrameIndex);
		copyToSwapchain(currentFrameIndex, currentSwapchainImageIndex);
		endRender(currentFrameIndex);

		{// Present
			RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
			m_device->GetGraphicsQueue()->QueuePresent(m_swapchain->GetSwapchain(), currentSwapchainImageIndex, m_semaphores.completeRender[currentFrameIndex]);
			m_lightmapBakingPipeline->OutputLightmap(m_resources, currentFrameIndex);
		}

		m_frameCounter++;
	}

	void RenderSubsystem::ResetWorld()
	{
		m_mainCamera->ResetCamera();
		m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
		m_objectRegistry->Clear();
	}

	void RenderSubsystem::Resize(const VkSurfaceKHR& surface, const glm::ivec2& screenSize)
	{
		m_stopRender = (screenSize == glm::ivec2(0));
		if (m_stopRender) return;

		// デバイスの処理を待つ
		VK_VALIDATION(vkDeviceWaitIdle(*m_device));
		VK_VALIDATION(vkResetFences(*m_device, AppInfo::g_maxInFlightFrameCount, m_fences.inFlightFrameFence));

		m_frameCounter = 0;

		// swapchain
		m_swapchain->Resize(surface);

		// resources
		m_resources->Resize(m_swapchain->GetScreenSize());

		// command buffers
		{// render
			m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.createVBufferPhase1);
			m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.createVBufferPhase2);
			m_device->GetGraphicsQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.render);
			m_commandBuffers.createVBufferPhase1 = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.createVBufferPhase2 = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.render = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
		}
		{// compute
			m_device->GetComputeQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.completeHiZPhase1);
			m_device->GetComputeQueue()->FreeCommandBuffers(*m_device, m_commandBuffers.completeHiZPhase2);
			m_commandBuffers.completeHiZPhase1 = m_device->GetComputeQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.completeHiZPhase2 = m_device->GetComputeQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
		}

		if (m_mainCamera)
		{
			m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
		}

		// Render Descriptors
		m_renderDescriptors->GetSceneInfoDescriptorSet()->UpdateScreenSize(m_swapchain->GetScreenSize());

		// Render Pass
		resizeRenderPass();

		// Render Pipeline
		resizeRenderPipeline();

		// semaphores/fences
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			vkDestroyFence(*m_device, m_fences.inFlightFrameFence[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeHiZPhase1[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeVBufferPhase1[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeHiZPhase2[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeVBufferPhase2[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.completeRender[i], m_device->GetAllocationCallbacks());
			vkDestroySemaphore(*m_device, m_semaphores.imageAvailable[i], m_device->GetAllocationCallbacks());
		}
		initializeSemaphore();
		initializeFence();
	}

	void RenderSubsystem::SetSkyboxCubemap(const std::array<FilePath, 6>& filePath)
	{
		m_skyboxPipeline->SetCubemap(m_resources, filePath);
	}

	void RenderSubsystem::createHiZDepth(const VkCommandBuffer commandBuffer, const uint32_t dstFrameIndex, const uint32_t srcFrameIndex, const VkSemaphore wait, const VkSemaphore signal)
	{
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			wait,		// wait
			signal);	// signal

		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.7f, 1.0f), "Hi-Z Depth Compute"));
		if (m_vbDispatchInfoCount > 0 && m_mainCamera)
		{
			m_hiZDepthComputePipeline->Dispatch(commandBuffer, dstFrameIndex, srcFrameIndex, m_swapchain->GetScreenSize());
		}
		m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		RenderQueue_ptr computeQueue = m_device->GetComputeQueue();
		computeQueue->Submit(submitInfo);
	}

	void RenderSubsystem::createVBufferPhase1(const uint32_t currentFrameIndex)
	{
		// このFrameのHi-ZDepthが終わったら、このFrameのVisibilityBufferが作れる
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
			m_semaphores.completeHiZPhase1[currentFrameIndex],		// wait
			m_semaphores.completeVBufferPhase1[currentFrameIndex]);	// signal

		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer commandBuffer = m_commandBuffers.createVBufferPhase1[currentFrameIndex];
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 1.0f), "MeshShader Render Phase1"));
		m_visibilityBufferPhase1RenderPass->BeginRender(commandBuffer, currentFrameIndex);
		if (m_vbDispatchInfoCount > 0 && m_mainCamera)
		{
			m_visibilityBufferPhase1Pipeline->BindCommandBuffer(commandBuffer, currentFrameIndex, m_vbDispatchInfoCount);
		}
		m_visibilityBufferPhase1RenderPass->EndRender(commandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		graphicsQueue->Submit(submitInfo);
	}

	void RenderSubsystem::createVBufferPhase2(const uint32_t currentFrameIndex)
	{
		// このFrameのHi-ZDepthが終わったら、このFrameのVisibilityBufferが作れる
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
			m_semaphores.completeHiZPhase2[currentFrameIndex],		// wait
			m_semaphores.completeVBufferPhase2[currentFrameIndex]);	// signal

		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer commandBuffer = m_commandBuffers.createVBufferPhase2[currentFrameIndex];
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 1.0f), "MeshShader Render Phase2"));
		m_visibilityBufferPhase2RenderPass->BeginRender(commandBuffer, currentFrameIndex);
		if (m_vbDispatchInfoCount > 0 && m_mainCamera)
		{
			m_visibilityBufferPhase2Pipeline->BindCommandBuffer(commandBuffer, currentFrameIndex);
		}
		m_visibilityBufferPhase2RenderPass->EndRender(commandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		graphicsQueue->Submit(submitInfo);
	}

	void RenderSubsystem::beginRender(const uint32_t currentFrameIndex)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer commandBuffer = m_commandBuffers.render[currentFrameIndex];
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
	}

	void RenderSubsystem::render(const uint32_t currentFrameIndex)
	{
		VkCommandBuffer commandBuffer = m_commandBuffers.render[currentFrameIndex];

		{// Skybox Render Pass
			m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.0f, 0.5f, 1.0f), "Skybox Render"));
			m_skyboxRenderPass->BeginRender(commandBuffer, currentFrameIndex);
			if (m_mainCamera)
			{
				m_skyboxPipeline->BindCommandBuffer(commandBuffer);
			}
			m_skyboxRenderPass->EndRender(commandBuffer);
			m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		}
		{// Lighting
			m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 0.0f), "Lighting Render"));
			m_lightingRenderPass->BeginRender(commandBuffer, currentFrameIndex);
			if (m_vbDispatchInfoCount > 0 && m_mainCamera)
			{
				m_lightingPipeline->BindCommandBuffer(commandBuffer, currentFrameIndex);
			}
			m_lightingRenderPass->EndRender(commandBuffer);
			m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		}
		{// PostProcess
			m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 0.5f, 0.0f), "Post Process Render"));
			m_postProcessRenderPass->BeginRender(commandBuffer, currentFrameIndex);
			if (m_mainCamera)
			{

			}
			m_postProcessRenderPass->EndRender(commandBuffer);
			m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		}
		{// LightMap
			if (m_lightmapBakingPipeline->IsBaking())
			{
				m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 0.0f, 0.0f), "Lightmap Baking Pass"));
				m_lightmapBakingPass->BeginRender(commandBuffer, currentFrameIndex);
				for (const MObject_ptr& object : *m_objects)
				{
					if (m_lightmapBakingPipeline->NextObject(m_resources, object))
					{
						m_lightmapBakingPipeline->BindCommandBuffer(commandBuffer);
					}
				}
				m_lightmapBakingPass->EndRender(commandBuffer);
				m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
			}
		}
		{// RayTracing
			m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.0f, 1.0f, 0.0f), "Ray Tracing Render"));
			bool hasMesh = m_vbDispatchInfoCount > 0;	// todo. もっといい判定方法を考える必要がある
			if (hasMesh && m_mainCamera)
			{
				m_rayTracingPipeline->BindCommandBuffer(commandBuffer, currentFrameIndex, m_swapchain->GetScreenSize());
			}
			m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		}
	}

	void RenderSubsystem::copyToSwapchain(const uint32_t currentFrameIndex, const uint32_t currentSwapchainImageIndex)
	{
		VkCommandBuffer commandBuffer = m_commandBuffers.render[currentFrameIndex];
		{// Copy Image To Swapchain Image
			CopyMainRenderTargetToSwapchainImage(commandBuffer, currentFrameIndex, currentSwapchainImageIndex);
			//CopyRayTracingRenderTargetToSwapchainImage(commandBuffer, currentFrameIndex, currentSwapchainImageIndex);
		}
	}

	void RenderSubsystem::endRender(const uint32_t currentFrameIndex)
	{
		VkCommandBuffer commandBuffer = m_commandBuffers.render[currentFrameIndex];
		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

		// これから書き込むSwapchain Imageの処理が終わったら、このFrameのMainRenderTargetが作れる
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			m_semaphores.imageAvailable[currentFrameIndex],		// wait
			m_semaphores.completeRender[currentFrameIndex]);	// signal

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		graphicsQueue->Submit(submitInfo, m_fences.inFlightFrameFence[currentFrameIndex]);
	}

	void RenderSubsystem::initializeRenderDevice(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		m_device = CreateRenderDevicePointer(instance);
	}

	void RenderSubsystem::initializeRenderSwapchain(const VkSurfaceKHR& surface)
	{
		m_swapchain = CreateRenderSwapchainPointer(m_device, surface);
	}

	void RenderSubsystem::initializeRenderDescriptors()
	{
		m_renderDescriptors = CreateRenderDescriptorsPointer(m_device);
		m_renderDescriptors->GetSceneInfoDescriptorSet()->UpdateScreenSize(m_swapchain->GetScreenSize());
	}

	void RenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateRenderResourcesPointer(m_device, m_renderDescriptors);
		m_resources->Initialize(m_swapchain->GetScreenSize());
	}

	void RenderSubsystem::initializeObjectRegistry()
	{
		m_objectRegistry = CreateMObjectRegistryPointer(m_device, m_resources, m_renderDescriptors);
		m_objects = m_objectRegistry->GetMObjectList();
	}

	void RenderSubsystem::initializeCommandPool()
	{
		// command pool
		{// render
			m_device->GetGraphicsQueue()->CreateCommandPool(*m_device, m_device->GetAllocationCallbacks());
			m_commandBuffers.createVBufferPhase1 = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.createVBufferPhase2 = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.render = m_device->GetGraphicsQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
		}
		{// compute
			m_device->GetComputeQueue()->CreateCommandPool(*m_device, m_device->GetAllocationCallbacks());
			m_commandBuffers.completeHiZPhase1 = m_device->GetComputeQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
			m_commandBuffers.completeHiZPhase2 = m_device->GetComputeQueue()->AllocateCommandBuffers(*m_device, AppInfo::g_maxInFlightFrameCount);
		}
		{// transfer
			m_device->GetTransferQueue()->CreateCommandPool(*m_device, m_device->GetAllocationCallbacks());
		}
	}

	void RenderSubsystem::initializeSemaphore()
	{
		// semaphore(present/render)
		VkSemaphoreCreateInfo semaphoreCreateInfo = Utility::Vulkan::CreateInfo::semaphoreCreateInfo();
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.completeHiZPhase1[i]));
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.completeVBufferPhase1[i]));
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.completeHiZPhase2[i]));
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.completeVBufferPhase2[i]));
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.completeRender[i]));
			VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.imageAvailable[i]));
		}

		// 最初のcompletePreRenderをsingleする
		const uint32_t previousFrameIndex = (m_frameCounter + 1) % AppInfo::g_maxInFlightFrameCount;
		VkSubmitInfo signalOnlySubmit{};
		signalOnlySubmit.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		signalOnlySubmit.signalSemaphoreCount = 1;
		signalOnlySubmit.pSignalSemaphores = &m_semaphores.completeVBufferPhase2[previousFrameIndex];
		// 何も実行せずにセマフォだけシグナル状態にする
		VK_VALIDATION(vkQueueSubmit(m_device->GetGraphicsQueue()->GetQueue(), 1, &signalOnlySubmit, VK_NULL_HANDLE));
	}

	void RenderSubsystem::initializeFence()
	{
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		fenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &m_fences.inFlightFrameFence[i]));
		}
	}

	void RenderSubsystem::initializeDebugUtils(const VkInstance& instance)
	{
		m_vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		m_vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
	}

	void RenderSubsystem::initializeRenderPass()
	{
		// Skybox Render Pass
		m_skyboxRenderPass = CreateSkyboxRenderPassPointer(m_device, m_resources, m_swapchain);
		m_skyboxRenderPass->Initialize();
		// Visibility Buffer Render Pass
		m_visibilityBufferPhase1RenderPass = CreateVisibilityBufferPhase1RenderPassPointer(m_device, m_resources, m_swapchain);
		m_visibilityBufferPhase1RenderPass->Initialize();
		m_visibilityBufferPhase2RenderPass = CreateVisibilityBufferPhase2RenderPassPointer(m_device, m_resources, m_swapchain);
		m_visibilityBufferPhase2RenderPass->Initialize();
		// Lighting Render Pass
		m_lightingRenderPass = CreateLightingRenderPassPointer(m_device, m_resources, m_swapchain);
		m_lightingRenderPass->Initialize();
		// PostProcess Render Pass
		m_postProcessRenderPass = CreatePostProcessRenderPassPointer(m_device, m_resources, m_swapchain);
		m_postProcessRenderPass->Initialize();
		// Lightmap Baking Pass
		m_lightmapBakingPass = CreateLightmapBakingPassPointer(m_device, m_resources, m_swapchain);
		m_lightmapBakingPass->Initialize();
	}

	void RenderSubsystem::initializeRenderPipeline()
	{
		// Skybox Pipeline
		m_skyboxPipeline = CreateSkyboxPipelinePointer(m_device, m_renderDescriptors);
		m_skyboxPipeline->Initialize(m_resources, m_skyboxRenderPass->GetRenderPass());
		// Visibility Buffer Pipeline
		m_visibilityBufferPhase1Pipeline = CreateVisibilityBufferPhase1PipelinePointer(m_device, m_renderDescriptors);
		m_visibilityBufferPhase1Pipeline->Initialize(m_resources, m_visibilityBufferPhase1RenderPass->GetRenderPass());
		m_visibilityBufferPhase2Pipeline = CreateVisibilityBufferPhase2PipelinePointer(m_device, m_renderDescriptors);
		m_visibilityBufferPhase2Pipeline->Initialize(m_resources, m_visibilityBufferPhase2RenderPass->GetRenderPass());
		// Lighting Pipeline
		m_lightingPipeline = CreateLightingPipelinePointer(m_device, m_renderDescriptors);
		m_lightingPipeline->Initialize(m_resources, m_lightingRenderPass->GetRenderPass());
		// Lightmap Baking Pipeline
		m_lightmapBakingPipeline = CreateLightmapBakingPipelinePointer(m_device, m_renderDescriptors);
		m_lightmapBakingPipeline->Initialize(m_resources, m_lightmapBakingPass->GetRenderPass());
		// RayTracing Pipeline
		m_rayTracingPipeline = CreateRayTracingPipelinePointer(m_device, m_renderDescriptors);
		m_rayTracingPipeline->Initialize(m_resources);
	}

	void RenderSubsystem::initializeComputePipeline()
	{
		m_hiZDepthComputePipeline = CreateHiZDepthComputePipelinePointer(m_device, m_resources, m_renderDescriptors);
		m_hiZDepthComputePipeline->Initialize();
	}

	void RenderSubsystem::initializeAccelerationStructureManager()
	{
		m_accelerationStructureManager = CreateAccelerationStructureManagerPointer(m_device, m_renderDescriptors, m_resources);
		m_resources->SetOnLoadedMesh([=](MeshesHandle& m) {m_accelerationStructureManager->OnLoadedMesh(m); });
		m_objectRegistry->SetOnAddObject([=](const MObject_ptr& m) {m_accelerationStructureManager->OnAddObject(m); });
	}

	void RenderSubsystem::resizeRenderPass()
	{
		m_skyboxRenderPass->Resize(m_swapchain->GetScreenSize());
		m_visibilityBufferPhase1RenderPass->Resize(m_swapchain->GetScreenSize());
		m_visibilityBufferPhase2RenderPass->Resize(m_swapchain->GetScreenSize());
		m_lightingRenderPass->Resize(m_swapchain->GetScreenSize());
		//m_lightmapBakingPass->Resize(m_swapchain->GetScreenSize());
	}

	void RenderSubsystem::resizeRenderPipeline()
	{
		m_hiZDepthComputePipeline->Resize();
		m_skyboxPipeline->Resize(m_resources);
		m_visibilityBufferPhase1Pipeline->Resize(m_resources);
		m_visibilityBufferPhase2Pipeline->Resize(m_resources);
		m_lightingPipeline->Resize(m_resources);
		//m_lightmapBakingPipeline->Resize(m_resources);
		m_rayTracingPipeline->Resize(m_resources);
	}

	void RenderSubsystem::CopyMainRenderTargetToSwapchainImage(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t swapchainImageIndex)
	{
		const Image& swapchainImage = m_swapchain->GetSwapchainImage()[swapchainImageIndex];
		const Image& mainRenderTarget = m_resources->GetMainRenderTarget(frameIndex);

		VkImageMemoryBarrier barrier{};
		{// SwapchainImage -> TRANSFER_SRC
			barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage.image;
			barrier.subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		// Copy
		VkImageBlit copyRegion = {};
		copyRegion.srcSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffsets[0] = { 0,0,0 };
		copyRegion.srcOffsets[1] = { m_swapchain->GetScreenSize().x, m_swapchain->GetScreenSize().y, 1 };
		copyRegion.dstSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffsets[0] = { 0,0,0 };
		copyRegion.dstOffsets[1] = { m_swapchain->GetScreenSize().x, m_swapchain->GetScreenSize().y, 1 };
		vkCmdBlitImage(commandBuffer,
			mainRenderTarget.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			swapchainImage.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion,
			VkFilter::VK_FILTER_NEAREST);
		{// SwapchainImage -> PRESENT_SRC
			barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage.image;
			barrier.subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	void RenderSubsystem::CopyRayTracingRenderTargetToSwapchainImage(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const uint32_t swapchainImageIndex)
	{
		const Image& swapchainImage = m_swapchain->GetSwapchainImage()[swapchainImageIndex];
		const Image& rayTracingRenderTarget = m_resources->GetRayTracingRenderTarget(frameIndex);

		VkImageMemoryBarrier barriers[2]{};
		{// RayTracingRenderTarget -> TRANSFER_DST
			barriers[0].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
			barriers[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
			barriers[0].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
			barriers[0].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = rayTracingRenderTarget.image;
			barriers[0].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		{// SwapchainImage -> TRANSFER_SRC
			barriers[1].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[1].srcAccessMask = 0;
			barriers[1].dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[1].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[1].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = swapchainImage.image;
			barriers[1].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barriers[0]);
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barriers[1]);
		// Copy
		VkImageBlit copyRegion = {};
		copyRegion.srcSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffsets[0] = { 0,0,0 };
		copyRegion.srcOffsets[1] = { m_swapchain->GetScreenSize().x, m_swapchain->GetScreenSize().y, 1 };
		copyRegion.dstSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffsets[0] = { 0,0,0 };
		copyRegion.dstOffsets[1] = { m_swapchain->GetScreenSize().x, m_swapchain->GetScreenSize().y, 1 };
		vkCmdBlitImage(commandBuffer,
			rayTracingRenderTarget.image, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL,
			swapchainImage.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion,
			VkFilter::VK_FILTER_NEAREST);
		{// RayTracingRenderTarget -> SHADER_WRITE
			barriers[0].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
			barriers[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
			barriers[0].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
			barriers[0].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = rayTracingRenderTarget.image;
			barriers[0].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		{// SwapchainImage -> PRESENT_SRC
			barriers[1].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[1].srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[1].dstAccessMask = 0;
			barriers[1].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barriers[1].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = swapchainImage.image;
			barriers[1].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			0,
			0, nullptr,
			0, nullptr,
			1, &barriers[0]);
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barriers[1]);
	}
}