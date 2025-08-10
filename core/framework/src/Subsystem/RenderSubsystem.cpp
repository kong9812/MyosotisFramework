// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <algorithm>
#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "StageObject.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"
#include "RenderResources.h"
#include "RenderDescriptors.h"

#include "DebugGUI.h"
#include "StaticMesh.h"
#include "FpsCamera.h"

#include "ShadowMapRenderPass.h"
#include "MainRenderPass.h"
#include "FinalCompositionRenderPass.h"
#include "MeshShaderRenderPass.h"

#include "SkyboxRenderPipeline.h"
#include "ShadowMapRenderPipeline.h"
#include "DeferredRenderPipeline.h"
#include "CompositionRenderPipeline.h"
#include "LightingRenderPipeline.h"
#include "FinalCompositionRenderPipeline.h"
#include "InteriorObjectDeferredRenderPipeline.h"
#include "MeshShaderRenderPhase1Pipeline.h"
#include "MeshShaderRenderPhase2Pipeline.h"

#include "HiZDepthComputePipeline.h"

#include "RenderQueue.h"
#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

#include "PrimitiveGeometry.h"
#include "Skybox.h"
#include "InteriorObject.h"
#include "RenderPieplineList.h"

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

	void RenderSubsystem::RegisterObject(const StageObject_ptr& object)
	{
		if (!m_mainCamera && object->HasCamera(true))
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
				if (IsStaticMesh(component->GetType()))
				{
					StaticMesh_ptr customMesh = Object_CastToStaticMesh(component);
					customMesh->PrepareForRender(m_device, m_resources);
					{// ShadowMap
						ShadowMapRenderPipeline::ShaderObject& shaderObject = customMesh->GetShadowMapShaderObject();
						m_shadowMapRenderPipeline->CreateShaderObject(shaderObject);
					}
					{// Deferred Render
						DeferredRenderPipeline::ShaderObject& shaderObject = customMesh->GetStaticMeshShaderObject();
						m_deferredRenderPipeline->CreateShaderObject(shaderObject);
					}
				}
			}
		}

		{// Skybox
			std::vector<ComponentBase_ptr> components = object->FindAllComponents(ComponentType::Skybox, true);
			for (ComponentBase_ptr& component : components)
			{
				if (component->GetType() == ComponentType::Skybox)
				{
					Skybox_ptr skybox = Object_CastToSkybox(component);
					skybox->PrepareForRender(m_device, m_resources);
					SkyboxRenderPipeline::ShaderObject& shaderObject = skybox->GetSkyboxShaderObject();
					m_skyboxRenderPipeline->CreateShaderObject(shaderObject);
				}
			}
		}

		{// InteriorObjectMesh
			std::vector<ComponentBase_ptr> components = object->FindAllComponents(ComponentType::InteriorObjectMesh, true);
			for (ComponentBase_ptr& component : components)
			{
				if (component->GetType() == ComponentType::InteriorObjectMesh)
				{
					InteriorObject_ptr interiorObject = Object_CastToInteriorObject(component);
					interiorObject->PrepareForRender(m_device, m_resources);
					InteriorObjectDeferredRenderPipeline::ShaderObject& shaderObject = interiorObject->GetInteriorObjectShaderObject();
					m_interiorObjectDeferredRenderPipeline->CreateShaderObject(shaderObject);
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
		if (m_mainCamera)
		{
			m_mainCamera->Update(updateData);
		}

		for (StageObject_ptr& object : m_objects)
		{
			object->Update(updateData, m_mainCamera);
		}
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
		submitInfo.pCommandBuffers = &m_computeCommandBuffers[0];		// Frustum Culling

		m_descriptors->UpdateMainDescriptorSet();

		RenderQueue_ptr computeQueue = m_device->GetComputeQueue();
		computeQueue->Submit(submitInfo);
		computeQueue->WaitIdle();

		m_descriptors->ResetMainDescriptorSet();
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

	void RenderSubsystem::ShadowRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// ShadowMap Prender Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.4f, 0.0f, 0.6f), "Shadow Render"));

		m_shadowMapRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_shadowMapRenderPass->EndRender(currentCommandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::MainRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		m_descriptors->UpdateMainCameraData(m_mainCamera->GetCameraData());
		for (StageObject_ptr& object : m_objects)
		{
			std::vector<ComponentBase_ptr> components = object->GetAllComponents(true);
			for (ComponentBase_ptr& component : components)
			{
				if (IsStaticMesh(component->GetType()))
				{
					StaticMesh_ptr staticMesh = Object_CastToStaticMesh(component);
					staticMesh->GetStaticMeshShaderObject().pushConstant.StandardSSBOIndex = m_descriptors->AddStandardSSBO(staticMesh->GetStaticMeshShaderObject().SSBO.standardSSBO);
				}
			}
		}

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// main render pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.8f, 0.8f, 1.0f), "Main Render"));

		m_mainRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 0.6f, 0.0f), "Lighting Render"));
		//m_lightingRenderPipeline->UpdateDescriptors(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		//m_lightingRenderPipeline->BindCommandBuffer(currentCommandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 0.0f), "Composition Render"));
		//m_compositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);

		m_mainRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::MeshShaderRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// mesh shader Render Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 1.0f, 1.0f), "MeshShader Render"));

		m_meshShaderRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_meshShaderRenderPhase1Pipeline->BindCommandBuffer(currentCommandBuffer, m_descriptors->GetStandardSSBOCount());

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_meshShaderRenderPhase2Pipeline->BindCommandBuffer(currentCommandBuffer, m_descriptors->GetStandardSSBOCount());

		m_meshShaderRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::FinalCompositionRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// final composition Render Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(1.0f, 0.0f, 1.0f), "FinalComposition Render"));

		m_finalCompositionRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_finalCompositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		m_finalCompositionRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::EndRender()
	{
		if (m_mainCamera == nullptr) return;
		if (m_objects.empty()) return;

		m_descriptors->UpdateMainDescriptorSet();

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkEndCommandBuffer(currentCommandBuffer));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &currentCommandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		VK_VALIDATION(vkResetFences(*m_device, 1, &m_renderFence));
		graphicsQueue->Submit(m_submitInfo, m_renderFence);
		m_swapchain->QueuePresent(graphicsQueue->GetQueue(), m_currentBufferIndex, m_semaphores.renderComplete);
		graphicsQueue->WaitIdle();

		m_descriptors->ResetMainDescriptorSet();
	}

	void RenderSubsystem::ResetGameStage()
	{
		m_mainCamera->ResetCamera();
		m_mainCamera->UpdateScreenSize(glm::vec2(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight())));
		m_objects.clear();
	}

	void RenderSubsystem::Resize(const VkSurfaceKHR& surface, const uint32_t& width, const uint32_t& height)
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
		m_descriptors = CreateRenderDescriptorsPointer(m_device);
		std::vector<std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>> data{};
		for (uint8_t i = 0; i < static_cast<uint8_t>(Shape::PrimitiveGeometryShape::Max); i++)
		{
			Shape::PrimitiveGeometryShape shape = static_cast<Shape::PrimitiveGeometryShape>(i);
			std::vector<Mesh> mesh{ Shape::createShape(shape) };
			data.push_back(std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>(shape, mesh));
		}
		m_descriptors->AddPrimitiveGeometry(data);
	}

	void RenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateRenderResourcesPointer(m_device, m_descriptors);
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
		m_shadowMapRenderPass = CreateShadowMapRenderPassPointer(m_device, m_resources, AppInfo::g_shadowMapSize, AppInfo::g_shadowMapSize);
		m_shadowMapRenderPass->Initialize();

		m_mainRenderPass = CreateMainRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_mainRenderPass->Initialize();

		m_finalCompositionRenderPass = CreateFinalCompositionRenderPassPointer(m_device, m_resources, m_swapchain);
		m_finalCompositionRenderPass->Initialize();

		m_meshShaderRenderPass = CreateMeshShaderRenderPassPointer(m_device, m_resources, m_swapchain);
		m_meshShaderRenderPass->Initialize();
	}

	void RenderSubsystem::initializeRenderPipeline()
	{
		m_shadowMapRenderPipeline = CreateShadowMapRenderPipelinePointer(m_device, m_descriptors);
		m_shadowMapRenderPipeline->Initialize(m_resources, m_shadowMapRenderPass->GetRenderPass());

		m_skyboxRenderPipeline = CreateSkyboxRenderPipelinePointer(m_device, m_descriptors);
		m_skyboxRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_deferredRenderPipeline = CreateDeferredRenderPipelinePointer(m_device, m_descriptors);
		m_deferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_lightingRenderPipeline = CreateLightingRenderPipelinePointer(m_device, m_descriptors);
		m_lightingRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_compositionRenderPipeline = CreateCompositionRenderPipelinePointer(m_device, m_descriptors);
		m_compositionRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_interiorObjectDeferredRenderPipeline = CreateInteriorObjectDeferredRenderPipelinePointer(m_device, m_descriptors);
		m_interiorObjectDeferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_finalCompositionRenderPipeline = CreateFinalCompositionRenderPipelinePointer(m_device, m_descriptors);
		m_finalCompositionRenderPipeline->Initialize(m_resources, m_finalCompositionRenderPass->GetRenderPass());

		m_lightingRenderPipeline->CreateShaderObject(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		m_lightingRenderPipeline->UpdateDirectionalLightInfo(m_shadowMapRenderPipeline->GetDirectionalLightInfo());
		m_compositionRenderPipeline->CreateShaderObject();
		m_finalCompositionRenderPipeline->CreateShaderObject();

		m_meshShaderRenderPhase1Pipeline = CreateMeshShaderRenderPhase1PipelinePointer(m_device, m_descriptors);
		m_meshShaderRenderPhase1Pipeline->Initialize(m_resources, m_meshShaderRenderPass->GetRenderPass());
		m_meshShaderRenderPhase2Pipeline = CreateMeshShaderRenderPhase2PipelinePointer(m_device, m_descriptors);
		m_meshShaderRenderPhase2Pipeline->Initialize(m_resources, m_meshShaderRenderPass->GetRenderPass());
	}

	void RenderSubsystem::initializeComputePipeline()
	{
		m_hiZDepthComputePipeline = CreateHiZDepthComputePipelinePointer(m_device, m_descriptors, m_resources);
		m_hiZDepthComputePipeline->Initialize();
	}

	void RenderSubsystem::resizeRenderPass(const uint32_t& width, const uint32_t& height)
	{
		m_shadowMapRenderPass->Resize(width, height);
		m_mainRenderPass->Resize(width, height);
		m_finalCompositionRenderPass->Resize(width, height);
	}
}