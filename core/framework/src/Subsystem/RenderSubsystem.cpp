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

#include "SkyboxRenderPipeline.h"
#include "ShadowMapRenderPipeline.h"
#include "DeferredRenderPipeline.h"
#include "CompositionRenderPipeline.h"
#include "LightingRenderPipeline.h"
#include "FinalCompositionRenderPipeline.h"
#include "InteriorObjectDeferredRenderPipeline.h"

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
		{// frustum culler
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.frustumPlanesUBO.buffer.buffer, m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.obbDatasSSBO.buffer.buffer, m_frustumCullerShaderObject.obbDatasSSBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.buffer, m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocation);
			vkDestroyPipeline(*m_device, m_frustumCullerShaderObject.shaderBase.pipeline, m_device->GetAllocationCallbacks());
			vkDestroyPipelineLayout(*m_device, m_frustumCullerShaderObject.shaderBase.pipelineLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorSetLayout(*m_device, m_frustumCullerShaderObject.shaderBase.descriptorSetLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		}

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
					StaticMeshShaderObject& shaderObject = customMesh->GetStaticMeshShaderObject();
					m_shadowMapRenderPipeline->CreateShaderObject(shaderObject);
					m_deferredRenderPipeline->CreateShaderObject(shaderObject);
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
					SkyboxShaderObject& shaderObject = skybox->GetSkyboxShaderObject();
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
					InteriorObjectShaderObject& shaderObject = interiorObject->GetInteriorObjectShaderObject();
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

		// Frustum Culler
		initializeFrustumCuller();

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
	}

	void RenderSubsystem::Update(const UpdateData& updateData)
	{
		if (m_mainCamera)
		{
			m_mainCamera->Update(updateData);
			m_lightingRenderPipeline->UpdateCameraPosition(glm::vec4(m_mainCamera->GetCameraPos(), 0.0f));
		}

		for (StageObject_ptr& object : m_objects)
		{
			object->Update(updateData, m_mainCamera);
		}
	}

	void RenderSubsystem::FrustumCuller()
	{
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.computeComplete);
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(m_computeCommandBuffers[0], &commandBufferBeginInfo));
			m_vkCmdBeginDebugUtilsLabelEXT(m_computeCommandBuffers[0], &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.5f, 0.1f, 0.1f), "Compute Pass"));
			vkCmdBindPipeline(m_computeCommandBuffers[0], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_frustumCullerShaderObject.shaderBase.pipeline);
			vkCmdBindDescriptorSets(m_computeCommandBuffers[0], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_frustumCullerShaderObject.shaderBase.pipelineLayout, 0, 1, &m_frustumCullerShaderObject.shaderBase.descriptorSet, 0, 0);

			uint32_t staticObjectCount = 0;
			{// Data setup
				{// frustumPlanesUBO
					glm::mat4 vp = m_mainCamera->GetProjectionMatrix() * m_mainCamera->GetViewMatrix();
					glm::vec4 rowX = glm::vec4(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
					glm::vec4 rowY = glm::vec4(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
					glm::vec4 rowZ = glm::vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
					glm::vec4 rowW = glm::vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[0] = normalizePlane(rowW + rowX); // Left
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[1] = normalizePlane(rowW - rowX); // Right
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[2] = normalizePlane(rowW + rowY); // Bottom
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[3] = normalizePlane(rowW - rowY); // Top
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[4] = normalizePlane(rowW + rowZ); // Near
					m_frustumCullerShaderObject.frustumPlanesUBO.data.planes[5] = normalizePlane(rowW - rowZ); // Far
				}
				{// obbDatasSSBO
					m_frustumCullerShaderObject.obbDatasSSBO.data.obbDatas.clear();
					for (StageObject_ptr& object : m_objects)
					{
						std::vector<ComponentBase_ptr> components = object->FindAllComponents(ComponentType::CustomMesh, true);
						std::vector<ComponentBase_ptr> subComponents = object->FindAllComponents(ComponentType::PrimitiveGeometryMesh, true);
						components.insert(components.end(), subComponents.begin(), subComponents.end());
						for (ComponentBase_ptr& component : components)
						{
							if (IsStaticMesh(component->GetType()))
							{
								StaticMesh_ptr staticMeshPtr = Object_CastToStaticMesh(component);
								m_frustumCullerShaderObject.obbDatasSSBO.data.obbDatas.push_back(staticMeshPtr->GetWorldOBBData());
								staticObjectCount++;
							}
						}
					}
				}
				{// visibleObjectsSSBO
					m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.clear();
					m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.resize(staticObjectCount);
				}
			}
			{// Update UBO/SSBO
				memcpy(m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocationInfo.pMappedData, &m_frustumCullerShaderObject.frustumPlanesUBO.data.planes, sizeof(m_frustumCullerShaderObject.frustumPlanesUBO.data.planes));
				memcpy(m_frustumCullerShaderObject.obbDatasSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.obbDatasSSBO.data.obbDatas.data(), staticObjectCount * sizeof(OBBData));
				memcpy(m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.data(), m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.size() * sizeof(uint32_t));
			}

			vkCmdDispatch(m_computeCommandBuffers[0], staticObjectCount, 1, 1);
			m_vkCmdEndDebugUtilsLabelEXT(m_computeCommandBuffers[0]);
			vkEndCommandBuffer(m_computeCommandBuffers[0]);
		}

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_computeCommandBuffers[0];		// Frustum Culler
		RenderQueue_ptr computeQueue = m_device->GetComputeQueue();
		computeQueue->Submit(submitInfo);
		computeQueue->WaitIdle();
	}

	void RenderSubsystem::BeginRender()
	{
		m_swapchain->AcquireNextImage(m_semaphores.presentComplete, m_currentBufferIndex);
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkBeginCommandBuffer(currentCommandBuffer, &commandBufferBeginInfo));
	}

	void RenderSubsystem::ShadowRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// ShadowMap Prender Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.1f, 0.5f), "ShadowMap Prender Pass"));

		m_shadowMapRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		for (StageObject_ptr& object : m_objects)
		{
			std::vector<ComponentBase_ptr> components = object->FindAllComponents(ComponentType::CustomMesh, true);
			std::vector<ComponentBase_ptr> subComponents = object->FindAllComponents(ComponentType::PrimitiveGeometryMesh, true);
			components.insert(components.end(), subComponents.begin(), subComponents.end());
			for (ComponentBase_ptr& component : components)
			{
				if (IsStaticMesh(component->GetType()))
				{
					StaticMesh_ptr staticMesh = Object_CastToStaticMesh(component);
					m_shadowMapRenderPipeline->UpdateDescriptors(staticMesh->GetStaticMeshShaderObject());
					staticMesh->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::ShadowMap);
				}
			}
		}

		m_shadowMapRenderPass->EndRender(currentCommandBuffer);
		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::MainRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// main render pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.8f, 0.1f), "Static Mesh Deferred Render Pass"));

		m_mainRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);
		memcpy(m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.data(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.size() * sizeof(uint32_t));

		std::vector<TransparentStaticMesh> staticMeshes{};
		uint32_t staticMeshCounter = 0;
		for (StageObject_ptr& object : m_objects)
		{
			std::vector<ComponentBase_ptr> components = object->GetAllComponents(true);
			for (ComponentBase_ptr& component : components)
			{
				if (IsStaticMesh(component->GetType()))
				{
					if (m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices[staticMeshCounter] == 1)
					{
						StaticMesh_ptr staticMesh = Object_CastToStaticMesh(component);

						// Zソート(簡易)
						staticMeshes.push_back({ m_mainCamera->GetDistance(staticMesh->GetPos()), staticMesh });
					}
					staticMeshCounter++;
				}
				else if (component->GetType() == ComponentType::Skybox)
				{
					Skybox_ptr staticMesh = Object_CastToSkybox(component);
					m_skyboxRenderPipeline->UpdateDescriptors(staticMesh->GetSkyboxShaderObject());
					staticMesh->BindCommandBuffer(currentCommandBuffer);
				}
				else if (component->GetType() == ComponentType::InteriorObjectMesh)
				{
					InteriorObject_ptr staticMesh = Object_CastToInteriorObject(component);
					m_interiorObjectDeferredRenderPipeline->UpdateDescriptors(staticMesh->GetInteriorObjectShaderObject());
					staticMesh->BindCommandBuffer(currentCommandBuffer);
				}
			}
		}

		// 距離ソート
		std::sort(staticMeshes.begin(), staticMeshes.end(),
			[](const TransparentStaticMesh& a, const TransparentStaticMesh& b) {
				return a.distance > b.distance;
			});

		{// Deferred Render Pass
			m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.2f, 0.1f), "Deferred SubPass"));
			for (TransparentStaticMesh& staticMeshesPair : staticMeshes)
			{
				m_deferredRenderPipeline->UpdateDescriptors(staticMeshesPair.staticMesh->GetStaticMeshShaderObject());
				staticMeshesPair.staticMesh->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::Deferred);
			}

			m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
		}

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_lightingRenderPipeline->UpdateDescriptors(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		m_lightingRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_compositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		m_mainRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::FinalCompositionRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// final composition Render Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.1f, 0.5f), "Final Composition Render Pass"));

		m_finalCompositionRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_finalCompositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		m_finalCompositionRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::EndRender()
	{
		m_descriptors->UpdateDescriptorSet();

		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkEndCommandBuffer(currentCommandBuffer));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &currentCommandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		VK_VALIDATION(vkResetFences(*m_device, 1, &m_renderFence));
		graphicsQueue->Submit(m_submitInfo, m_renderFence);
		m_swapchain->QueuePresent(graphicsQueue->GetQueue(), m_currentBufferIndex, m_semaphores.renderComplete);
		graphicsQueue->WaitIdle();

		m_descriptors->ResetInfos();
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
			m_computeCommandBuffers.resize(1);	// Frustum Culler
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

			m_computeCommandBuffers.resize(1);	// Frustum Culler
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_computeCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_computeCommandBuffers.data()));
		}
		{// transfer
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetTransferQueue()->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_transferCommandPool));
		}
	}

	void RenderSubsystem::initializeFrustumCuller()
	{
		{// frustumPlanesUBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.frustumPlanesUBO.data,
				static_cast<uint32_t>(sizeof(m_frustumCullerShaderObject.frustumPlanesUBO.data.planes)),
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				m_frustumCullerShaderObject.frustumPlanesUBO.buffer.buffer,
				m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocation,
				m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocationInfo,
				m_frustumCullerShaderObject.frustumPlanesUBO.buffer.descriptor);
		}
		{// objectMinSSBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.obbDatasSSBO.data,
				static_cast<uint32_t>(sizeof(OBBData)) * AppInfo::g_maxObject,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				m_frustumCullerShaderObject.obbDatasSSBO.buffer.buffer,
				m_frustumCullerShaderObject.obbDatasSSBO.buffer.allocation,
				m_frustumCullerShaderObject.obbDatasSSBO.buffer.allocationInfo,
				m_frustumCullerShaderObject.obbDatasSSBO.buffer.descriptor);
		}
		{// visibleObjectsSSBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.visibleObjectsSSBO.data,
				static_cast<uint32_t>(sizeof(uint32_t)) * AppInfo::g_maxObject,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.buffer,
				m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocation,
				m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo,
				m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.descriptor);
		}

		{// m_frustumDescriptor
			// pool
			std::vector<VkDescriptorPoolSize> poolSize = {
				Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
				Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3),
			};
			VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, 1);
			VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

			// [descriptor]layout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
				// binding: [ubo] 0
				Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT),
				// binding: [ssbo] 1
				Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT),
				// binding: [ssbo] 2
				Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(2, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT),
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
			VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_frustumCullerShaderObject.shaderBase.descriptorSetLayout));
			// layout allocate
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_frustumCullerShaderObject.shaderBase.descriptorSetLayout);
			VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_frustumCullerShaderObject.shaderBase.descriptorSet));
			// write descriptor set
			std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_frustumCullerShaderObject.shaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_frustumCullerShaderObject.frustumPlanesUBO.buffer.descriptor),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_frustumCullerShaderObject.shaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_frustumCullerShaderObject.obbDatasSSBO.buffer.descriptor),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_frustumCullerShaderObject.shaderBase.descriptorSet, 2, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.descriptor),
			};
			vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
		}

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_frustumCullerShaderObject.shaderBase.descriptorSetLayout };
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_frustumCullerShaderObject.shaderBase.pipelineLayout));
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, m_resources->GetShaderModules("Frustum_Culling.comp.spv"));
		VkComputePipelineCreateInfo computePipelineCreateInfo = Utility::Vulkan::CreateInfo::computePipelineCreateInfo(m_frustumCullerShaderObject.shaderBase.pipelineLayout, shaderStageCreateInfo);
		VK_VALIDATION(vkCreateComputePipelines(*m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_frustumCullerShaderObject.shaderBase.pipeline));
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
	}

	void RenderSubsystem::resizeRenderPass(const uint32_t& width, const uint32_t& height)
	{
		m_shadowMapRenderPass->Resize(width, height);
		m_mainRenderPass->Resize(width, height);
		m_finalCompositionRenderPass->Resize(width, height);
	}
}