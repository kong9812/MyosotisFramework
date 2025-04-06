// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <algorithm>
#include <array>
#include <backends/imgui_impl_vulkan.h>

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
	RenderSubsystem::RenderSubsystem(const GLFWwindow& glfwWindow, const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		m_instance = instance;
		m_onPressedSaveGameStageCallback = nullptr;

		// RenderDevice
		m_device = CreateRenderDevicePointer(m_instance);

		// Swapchain
		m_swapchain = CreateRenderSwapchainPointer(m_device, surface);

		// Resources
		m_resources = CreateRenderResourcesPointer(m_device, m_swapchain->GetWidth(), m_swapchain->GetHeight());

		// graphics queue
		vkGetDeviceQueue(*m_device, m_device->GetGraphicsFamilyIndex(), 0, &m_graphicsQueue);
		// compute queue
		vkGetDeviceQueue(*m_device, m_device->GetComputeFamilyIndex(), 0, &m_computeQueue);

		// command pool
		{// render
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetGraphicsFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_renderCommandPool));
		}
		{// compute
			VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetComputeFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_computeCommandPool));
		}
		prepareCommandBuffers();

		// fences
		prepareFences();

		// frustum culler
		prepareFrustumCuller();

		// semaphore(present/render)
		VkSemaphoreCreateInfo semaphoreCreateInfo = Utility::Vulkan::CreateInfo::semaphoreCreateInfo();
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.presentComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.computeComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.renderComplete));

		// submit info
		m_submitPipelineStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		m_submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.computeComplete, m_semaphores.renderComplete);

		m_vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		m_vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));

		m_shadowMapRenderPass = CreateShadowMapRenderPassPointer(m_device, m_resources, AppInfo::g_shadowMapSize, AppInfo::g_shadowMapSize);
		m_mainRenderPass = CreateMainRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_finalCompositionRenderPass = CreateFinalCompositionRenderPassPointer(m_device, m_resources, m_swapchain);

		m_skyboxRenderPipeline = CreateSkyboxRenderPipelinePointer(m_device, m_resources, m_mainRenderPass->GetRenderPass());
		m_shadowMapRenderPipeline = CreateShadowMapRenderPipelinePointer(m_device, m_resources, m_shadowMapRenderPass->GetRenderPass());
		m_deferredRenderPipeline = CreateDeferredRenderPipelinePointer(m_device, m_resources, m_mainRenderPass->GetRenderPass());
		m_compositionRenderPipeline = CreateCompositionRenderPipelinePointer(m_device, m_resources, m_mainRenderPass->GetRenderPass());
		m_compositionRenderPipeline->CreateShaderObject(m_resources->GetLightingResult());
		m_lightingRenderPipeline = CreateLightingRenderPipelinePointer(m_device, m_resources, m_mainRenderPass->GetRenderPass());
		m_lightingRenderPipeline->CreateShaderObject(m_resources->GetPosition(), m_resources->GetNormal(), m_resources->GetBaseColor(), m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		m_lightingRenderPipeline->UpdateDirectionalLightInfo(m_shadowMapRenderPipeline->GetDirectionalLightInfo());
		m_finalCompositionRenderPipeline = CreateFinalCompositionRenderPipelinePointer(m_device, m_resources, m_finalCompositionRenderPass->GetRenderPass());
		m_finalCompositionRenderPipeline->CreateShaderObject(m_resources->GetMainRenderTarget());
		m_interiorObjectDeferredRenderPipeline = CreateInteriorObjectDeferredRenderPipelinePointer(m_device, m_resources, m_mainRenderPass->GetRenderPass());
	}

	RenderSubsystem::~RenderSubsystem()
	{
		{// furstum culler
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.frustumPlanesUBO.buffer.buffer, m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.objectDataSSBO.buffer.buffer, m_frustumCullerShaderObject.objectDataSSBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.buffer, m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocation);
			vkDestroyPipeline(*m_device, m_frustumCullerShaderObject.shaderBase.pipeline, m_device->GetAllocationCallbacks());
			vkDestroyPipelineLayout(*m_device, m_frustumCullerShaderObject.shaderBase.pipelineLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorSetLayout(*m_device, m_frustumCullerShaderObject.shaderBase.descriptorSetLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		}

		vkDestroySemaphore(*m_device, m_semaphores.presentComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.computeComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.renderComplete, m_device->GetAllocationCallbacks());
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		vkDestroyCommandPool(*m_device, m_renderCommandPool, m_device->GetAllocationCallbacks());
		vkDestroyCommandPool(*m_device, m_computeCommandPool, m_device->GetAllocationCallbacks());
	}

	void RenderSubsystem::ResetMousePos(const glm::vec2& mousePos)
	{
		if (!m_mainCamera) return;

		Object_Cast<Camera::FPSCamera>(m_mainCamera)->ResetMousePos(mousePos);
	}

	void RenderSubsystem::RegisterObject(const ObjectBase_ptr& object)
	{
		switch (object->GetObjectType())
		{
		case ObjectType::FPSCamera:
		{
			if (!m_mainCamera)
			{
				m_mainCamera = Camera::Object_CastToCameraBase(object);
			}
		}
		break;
		case ObjectType::PrimitiveGeometryMesh:
		case ObjectType::CustomMesh:
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);
			staticMesh->PrepareForRender(m_device, m_resources);
			StaticMeshShaderObject& shaderObject = staticMesh->GetStaticMeshShaderObject();
			m_shadowMapRenderPipeline->CreateShaderObject(shaderObject);
			//m_transparentRenderPipeline->CreateShaderObject(shaderObject);
			m_deferredRenderPipeline->CreateShaderObject(shaderObject);
		}
		break;
		case ObjectType::Skybox:
		{
			Skybox_ptr skybox = Object_CastToSkybox(object);
			skybox->PrepareForRender(m_device, m_resources);
			SkyboxShaderObject& shaderObject = skybox->GetSkyboxShaderObject();
			m_skyboxRenderPipeline->CreateShaderObject(shaderObject);
		}
		break;
		case ObjectType::InteriorObjectMesh:
		{
			InteriorObject_ptr interiorObject = Object_CastToInteriorObject(object);
			interiorObject->PrepareForRender(m_device, m_resources);
			InteriorObjectShaderObject& shaderObject = interiorObject->GetInteriorObjectShaderObject();
			m_interiorObjectDeferredRenderPipeline->CreateShaderObject(shaderObject);
		}
		break;
		default:
			break;
		}
		m_objects.push_back(object);
	}

	void RenderSubsystem::Update(const UpdateData& updateData)
	{
		if (m_mainCamera)
		{
			Camera::FPSCamera_ptr cameraPtr = Object_Cast<Camera::FPSCamera>(m_mainCamera);
			cameraPtr->Update(updateData);
			m_lightingRenderPipeline->UpdateCameraPosition(glm::vec4(cameraPtr->GetCameraPos(), 0.0f));
		}

		std::vector<StaticMesh_ptr> firstStaticMesh{};
		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);
				staticMesh->Update(updateData, m_mainCamera);
				firstStaticMesh.push_back(staticMesh);
			}
			else if (object->GetObjectType() == ObjectType::Skybox)
			{
				Skybox_ptr staticMesh = Object_CastToSkybox(object);
				staticMesh->Update(updateData, m_mainCamera);
			}
			else if (object->GetObjectType() == ObjectType::InteriorObjectMesh)
			{
				InteriorObject_ptr staticMesh = Object_CastToInteriorObject(object);
				staticMesh->Update(updateData, m_mainCamera);
			}
		}

		// TEST
		g_debugTimer += updateData.deltaTime * g_debugSpeed;
		if (g_debugTimer >= 360.0f)
		{
			g_debugTimer -= 360.0f;
		}
		firstStaticMesh[1]->SetRot(glm::vec3(g_debugTimer, 0.0f, 0.0f));
		firstStaticMesh[1]->Update(updateData, m_mainCamera);
		// TEST
	}

	void RenderSubsystem::Compute()
	{
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.computeComplete);
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(m_computeCommandBuffers[0], &commandBufferBeginInfo));
			m_vkCmdBeginDebugUtilsLabelEXT(m_computeCommandBuffers[0], &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.5f, 0.1f, 0.1f), "Compute Pass"));
			vkCmdBindPipeline(m_computeCommandBuffers[0], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_frustumCullerShaderObject.shaderBase.pipeline);
			vkCmdBindDescriptorSets(m_computeCommandBuffers[0], VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_frustumCullerShaderObject.shaderBase.pipelineLayout, 0, 1, &m_frustumCullerShaderObject.shaderBase.descriptorSet, 0, 0);

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
				{// objectDataSSBO
					m_frustumCullerShaderObject.objectDataSSBO.data.objects.clear();
					for (ObjectBase_ptr& object : m_objects)
					{
						if (IsStaticMesh(object->GetObjectType()))
						{
							StaticMesh_ptr staticMeshPtr = Object_CastToStaticMesh(object);
							m_frustumCullerShaderObject.objectDataSSBO.data.objects.push_back(staticMeshPtr->GetCullerData());
						}
					}
				}
				{// visibleObjectsSSBO
					m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.clear();
					m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.resize(m_frustumCullerShaderObject.objectDataSSBO.data.objects.size());
				}
			}
			{// Update UBO/SSBO
				memcpy(m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocationInfo.pMappedData, &m_frustumCullerShaderObject.frustumPlanesUBO.data.planes, sizeof(m_frustumCullerShaderObject.frustumPlanesUBO.data.planes));
				memcpy(m_frustumCullerShaderObject.objectDataSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.objectDataSSBO.data.objects.data(), m_frustumCullerShaderObject.objectDataSSBO.data.objects.size() * sizeof(glm::vec4));
				memcpy(m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.data(), m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.size() * sizeof(uint32_t));
			}

			vkCmdDispatch(m_computeCommandBuffers[0], m_frustumCullerShaderObject.objectDataSSBO.data.objects.size(), 1, 1);
			m_vkCmdEndDebugUtilsLabelEXT(m_computeCommandBuffers[0]);
			vkEndCommandBuffer(m_computeCommandBuffers[0]);
		}

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_computeCommandBuffers[0];		// Frustum Culler
		VK_VALIDATION(vkQueueSubmit(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_VALIDATION(vkQueueWaitIdle(m_computeQueue));
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

		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				Object_CastToStaticMesh(object)->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::ShadowMap);
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
		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				//if (m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices[staticMeshCounter] == 1)
				//{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);

				// Zソート(簡易)
				staticMeshes.push_back({ m_mainCamera->GetDistance(staticMesh->GetPos()), staticMesh });
				//}
				staticMeshCounter++;
			}
			else if (object->GetObjectType() == ObjectType::Skybox)
			{
				Skybox_ptr staticMesh = Object_CastToSkybox(object);
				staticMesh->BindCommandBuffer(currentCommandBuffer);
			}
			else if (object->GetObjectType() == ObjectType::InteriorObjectMesh)
			{
				InteriorObject_ptr staticMesh = Object_CastToInteriorObject(object);
				staticMesh->BindCommandBuffer(currentCommandBuffer);
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
				staticMeshesPair.staticMesh->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::Deferred);
				Logger::Info(std::string("rending: ") + std::to_string(staticMeshes.size()));
			}

			m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
		}

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_lightingRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_compositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		m_mainRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::FinalCompositionRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];

		// final composition Prender Pass
		m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.1f, 0.5f), "Final Composition Prender Pass"));

		m_finalCompositionRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);

		m_finalCompositionRenderPipeline->BindCommandBuffer(currentCommandBuffer);

		m_finalCompositionRenderPass->EndRender(currentCommandBuffer);

		m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
	}

	void RenderSubsystem::EndRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkEndCommandBuffer(currentCommandBuffer));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &currentCommandBuffer;
		VK_VALIDATION(vkQueueSubmit(m_graphicsQueue, 1, &m_submitInfo, VK_NULL_HANDLE));
		m_swapchain->QueuePresent(m_graphicsQueue, m_currentBufferIndex, m_semaphores.renderComplete);
		VK_VALIDATION(vkQueueWaitIdle(m_graphicsQueue));
	}

	void RenderSubsystem::Resize(const VkSurfaceKHR& surface, const uint32_t& width, const uint32_t& height)
	{
		// デバイスの処理を待つ
		vkDeviceWaitIdle(*m_device);

		// swapchain
		m_swapchain.reset();
		m_swapchain = CreateRenderSwapchainPointer(m_device, surface);

		// command buffers
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		prepareCommandBuffers();

		// fences
		for (VkFence& fence : m_fences)
		{
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		prepareFences();

		if (m_mainCamera)
		{
			m_mainCamera->UpdateScreenSize(glm::vec2(width, height));
		}

		vkDeviceWaitIdle(*m_device);
	}

	void RenderSubsystem::prepareCommandBuffers()
	{
		{// graphics
			m_renderCommandBuffers.resize(m_swapchain->GetImageCount());
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_renderCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_swapchain->GetImageCount()));
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_renderCommandBuffers.data()));
		}
		{// compute
			m_computeCommandBuffers.resize(1);	// Frustum Culler
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_computeCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_computeCommandBuffers.data()));
		}
	}

	void RenderSubsystem::prepareFences()
	{
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo(VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT);
		m_fences.resize(m_renderCommandBuffers.size());
		for (VkFence& fence : m_fences) {
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));
		}
	}

	void RenderSubsystem::prepareFrustumCuller()
	{
		// todo.　動的に変更できるように
		const uint32_t staticMeshCount = 100;

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
		{// objectDataSSBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.objectDataSSBO.data,
				static_cast<uint32_t>(sizeof(glm::vec4)) * staticMeshCount,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				m_frustumCullerShaderObject.objectDataSSBO.buffer.buffer,
				m_frustumCullerShaderObject.objectDataSSBO.buffer.allocation,
				m_frustumCullerShaderObject.objectDataSSBO.buffer.allocationInfo,
				m_frustumCullerShaderObject.objectDataSSBO.buffer.descriptor);
		}
		{// visibleObjectsSSBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.visibleObjectsSSBO.data,
				static_cast<uint32_t>(sizeof(uint32_t)) * staticMeshCount,
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
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_frustumCullerShaderObject.shaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_frustumCullerShaderObject.objectDataSSBO.buffer.descriptor),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_frustumCullerShaderObject.shaderBase.descriptorSet, 2, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.descriptor),
			};
			vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_frustumCullerShaderObject.shaderBase.descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_frustumCullerShaderObject.shaderBase.pipelineLayout));
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, m_resources->GetShaderModules("Frustum_Culling.comp.spv"));
		VkComputePipelineCreateInfo computePipelineCreateInfo = Utility::Vulkan::CreateInfo::computePipelineCreateInfo(m_frustumCullerShaderObject.shaderBase.pipelineLayout, shaderStageCreateInfo);
		VK_VALIDATION(vkCreateComputePipelines(*m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_frustumCullerShaderObject.shaderBase.pipeline));
	}

	void RenderSubsystem::SetOnPressedSaveGameStageCallback(const OnPressedSaveGameStageCallback& callback)
	{
		m_onPressedSaveGameStageCallback = callback;
	}
	void RenderSubsystem::SetOnPressedLoadGameStageCallback(const OnPressedLoadGameStageCallback& callback)
	{
		m_onPressedLoadGameStageCallback = callback;
	}
	void RenderSubsystem::SetOnPressedCreateObjectCallback(const OnPressedCreateObjectCallback& callback)
	{
		m_onPressedCreateObjectCallback = callback;
	}
}