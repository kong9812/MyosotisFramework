// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <algorithm>
#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "MObject.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"
#include "RenderResources.h"
#include "RenderDescriptors.h"

#include "DescriptorPool.h"
#include "SceneInfoDescriptorSet.h"
#include "MeshInfoDescriptorSet.h"
#include "ObjectInfoDescriptorSet.h"
#include "TextureDescriptorSet.h"

#include "DebugGUI.h"
#include "StaticMesh.h"
#include "FpsCamera.h"

#include "MeshShaderRenderPass.h"

#include "VisibilityBufferRenderPhase1Pipeline.h"
#include "VisibilityBufferRenderPhase2Pipeline.h"

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
		vkDestroyFence(*m_device, m_renderFence, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.presentComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.computeComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.renderComplete, m_device->GetAllocationCallbacks());
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		vkDestroyCommandPool(*m_device, m_renderCommandPool, m_device->GetAllocationCallbacks());
		vkDestroyCommandPool(*m_device, m_computeCommandPool, m_device->GetAllocationCallbacks());
		vkDestroyCommandPool(*m_device, m_transferCommandPool, m_device->GetAllocationCallbacks());
	}

	void RenderSubsystem::ResetMousePos(const glm::vec2& mousePos)
	{
		if (!m_mainCamera) return;

		Object_Cast<Camera::FPSCamera>(m_mainCamera)->ResetMousePos(mousePos);
	}

	void RenderSubsystem::RegisterObject(const MObject_ptr& object)
	{
		if (!m_mainCamera && object->IsCamera(true))
		{
			m_mainCamera = Camera::Object_CastToCameraBase(object->FindComponent(ComponentType::FPSCamera, true));
			m_mainCamera->UpdateScreenSize(glm::vec2(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight())));
		}

		{// StaticMesh
			std::vector<ComponentBase_ptr> components = object->FindAllComponents(ComponentType::CustomMesh, true);
			std::vector<ComponentBase_ptr> subComponents = object->FindAllComponents(ComponentType::PrimitiveGeometryMesh, true);
			components.insert(components.end(), subComponents.begin(), subComponents.end());
			for (ComponentBase_ptr& component : components)
			{
				if (component->IsStaticMesh())
				{
					StaticMesh_ptr customMesh = Object_CastToStaticMesh(component);
					customMesh->PrepareForRender(m_device, m_resources, m_meshInfoDescriptorSet);
				}
			}
		}

		m_objects.push_back(object);
	}

	void RenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		// Vulkan Instance
		initializeRenderDevice(instance, surface);

		// Swapchain
		initializeRenderSwapchain(surface);

		// Descriptors
		initializeRenderDescriptors();

		// Resources
		initializeRenderResources();

		// Command pool
		initializeCommandPool();

		// Semaphore
		initializeSemaphore();

		// Fence
		initializeFence();

		// Submit info
		initializeSubmitInfo();

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
		m_meshletCount = 0;

		if (m_mainCamera)
		{
			m_mainCamera->Update(updateData);
		}

		for (MObject_ptr& object : m_objects)
		{
			object->Update(updateData, m_mainCamera);
			m_objectInfoDescriptorSet->AddObjectInfo(object->GetObjectInfo());

			std::vector<VBDispatchInfo> vbDispatchInfo = object->GetVBDispatchInfo();
			m_objectInfoDescriptorSet->AddVBDispatchInfo(vbDispatchInfo);

			for (const VBDispatchInfo& info : vbDispatchInfo)
			{
				MeshInfo meshInfo = m_meshInfoDescriptorSet->GetMeshInfo(info.meshID);
				m_meshletCount += meshInfo.meshletCount;
			}
		}

		m_sceneInfoDescriptorSet->Update();
		m_objectInfoDescriptorSet->Update();
		m_meshInfoDescriptorSet->Update();
		m_textureDescriptorSet->Update();
	}

	void RenderSubsystem::BeginCompute()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.computeComplete);
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(m_computeCommandBuffers[0], &commandBufferBeginInfo));
			m_vkCmdBeginDebugUtilsLabelEXT(m_computeCommandBuffers[0], &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.7f, 1.0f), "Hi-Z Depth Compute"));

			m_hiZDepthComputePipeline->Dispatch(m_computeCommandBuffers[0], m_swapchain->GetScreenSize());

			m_vkCmdEndDebugUtilsLabelEXT(m_computeCommandBuffers[0]);
			VK_VALIDATION(vkEndCommandBuffer(m_computeCommandBuffers[0]));
		}
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_computeCommandBuffers[0];

		RenderQueue_ptr computeQueue = m_device->GetComputeQueue();
		computeQueue->Submit(submitInfo);
		computeQueue->WaitIdle();
	}

	void RenderSubsystem::BeginRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		m_swapchain->AcquireNextImage(m_semaphores.presentComplete, m_currentBufferIndex);
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkBeginCommandBuffer(currentCommandBuffer, &commandBufferBeginInfo));
	}

	void RenderSubsystem::MeshShaderRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// mesh shader Render Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 1.0f), "MeshShader Render"));

		m_meshShaderRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_visibilityBufferRenderPhase1Pipeline->BindCommandBuffer(currentCommandBuffer,
			m_meshletCount);

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_visibilityBufferRenderPhase2Pipeline->BindCommandBuffer(currentCommandBuffer,
			m_meshletCount);

		m_meshShaderRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::EndRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkEndCommandBuffer(currentCommandBuffer));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &currentCommandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		VK_VALIDATION(vkResetFences(*m_device, 1, &m_renderFence));
		graphicsQueue->Submit(m_submitInfo, m_renderFence);
		m_swapchain->QueuePresent(graphicsQueue->GetQueue(), m_currentBufferIndex, m_semaphores.renderComplete);
		graphicsQueue->WaitIdle();
	}

	void RenderSubsystem::ResetGameStage()
	{
		m_mainCamera->ResetCamera();
		m_mainCamera->UpdateScreenSize(glm::vec2(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight())));
		m_objects.clear();
	}

	void RenderSubsystem::Resize(const VkSurfaceKHR& surface, const uint32_t width, const uint32_t height)
	{
		// デバイスの処理を待つ
		VK_VALIDATION(vkResetFences(*m_device, 1, &m_renderFence));
		m_device->GetGraphicsQueue()->WaitIdle();
		m_device->GetComputeQueue()->WaitIdle();
		m_device->GetTransferQueue()->WaitIdle();
		VK_VALIDATION(vkDeviceWaitIdle(*m_device));

		// swapchain
		m_swapchain->Resize(surface);

		// resources
		m_resources->Resize(width, height);

		// command buffers
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		{// render
			m_renderCommandBuffers.resize(m_swapchain->GetImageCount());
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_renderCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_swapchain->GetImageCount()));
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_renderCommandBuffers.data()));
		}
		{// compute
			m_computeCommandBuffers.resize(1);	// Frustum Culling
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_computeCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_computeCommandBuffers.data()));
		}

		if (m_mainCamera)
		{
			m_mainCamera->UpdateScreenSize(glm::vec2(width, height));
		}

		// Render Pass
		resizeRenderPass(width, height);

		m_submitPipelineStages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		m_currentBufferIndex = 0;

		VK_VALIDATION(vkDeviceWaitIdle(*m_device));
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
		m_descriptorPool = CreateDescriptorPoolPointer(m_device);
		m_sceneInfoDescriptorSet = CreateSceneInfoDescriptorSetPointer(m_device, m_descriptorPool->GetDescriptorPool());
		m_objectInfoDescriptorSet = CreateObjectInfoDescriptorSetPointer(m_device, m_descriptorPool->GetDescriptorPool());
		m_meshInfoDescriptorSet = CreateMeshInfoDescriptorSetPointer(m_device, m_descriptorPool->GetDescriptorPool());
		m_textureDescriptorSet = CreateTextureDescriptorSetPointer(m_device, m_descriptorPool->GetDescriptorPool());
	}

	void RenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateRenderResourcesPointer(m_device);
		m_resources->Initialize(m_swapchain->GetWidth(), m_swapchain->GetHeight());
	}

	void RenderSubsystem::initializeCommandPool()
	{
		// command pool
		{// render
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetGraphicsQueue()->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_renderCommandPool));

			m_renderCommandBuffers.resize(m_swapchain->GetImageCount());
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_renderCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_swapchain->GetImageCount()));
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_renderCommandBuffers.data()));
		}
		{// compute
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetComputeQueue()->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_computeCommandPool));

			m_computeCommandBuffers.resize(1);	// Frustum Culling
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_computeCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_computeCommandBuffers.data()));
		}
		{// transfer
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetTransferQueue()->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_transferCommandPool));
		}
	}

	void RenderSubsystem::initializeSemaphore()
	{
		// semaphore(present/render)
		VkSemaphoreCreateInfo semaphoreCreateInfo = Utility::Vulkan::CreateInfo::semaphoreCreateInfo();
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.presentComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.computeComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.renderComplete));
	}

	void RenderSubsystem::initializeFence()
	{
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &m_renderFence));
	}

	void RenderSubsystem::initializeSubmitInfo()
	{
		m_submitPipelineStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		m_submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.computeComplete, m_semaphores.renderComplete);
	}

	void RenderSubsystem::initializeDebugUtils(const VkInstance& instance)
	{
		m_vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		m_vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
	}

	void RenderSubsystem::initializeRenderPass()
	{
		m_meshShaderRenderPass = CreateMeshShaderRenderPassPointer(m_device, m_resources, m_swapchain);
		m_meshShaderRenderPass->Initialize();
	}

	void RenderSubsystem::initializeRenderPipeline()
	{
		m_visibilityBufferRenderPhase1Pipeline = CreateVisibilityBufferRenderPhase1PipelinePointer(m_device,
			m_sceneInfoDescriptorSet, m_objectInfoDescriptorSet,
			m_meshInfoDescriptorSet, m_textureDescriptorSet);
		m_visibilityBufferRenderPhase1Pipeline->Initialize(m_resources, m_meshShaderRenderPass->GetRenderPass());
		m_visibilityBufferRenderPhase2Pipeline = CreateVisibilityBufferRenderPhase2PipelinePointer(m_device,
			m_sceneInfoDescriptorSet, m_objectInfoDescriptorSet,
			m_meshInfoDescriptorSet, m_textureDescriptorSet);
		m_visibilityBufferRenderPhase2Pipeline->Initialize(m_resources, m_meshShaderRenderPass->GetRenderPass());
	}

	void RenderSubsystem::initializeComputePipeline()
	{
		m_hiZDepthComputePipeline = CreateHiZDepthComputePipelinePointer(
			m_device, m_resources,
			m_sceneInfoDescriptorSet, m_objectInfoDescriptorSet,
			m_meshInfoDescriptorSet, m_textureDescriptorSet);
		m_hiZDepthComputePipeline->Initialize();
	}

	void RenderSubsystem::resizeRenderPass(const uint32_t width, const uint32_t height)
	{
		//m_shadowMapRenderPass->Resize(width, height);
		//m_mainRenderPass->Resize(width, height);
		//m_finalCompositionRenderPass->Resize(width, height);
	}
}