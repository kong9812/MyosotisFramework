// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <algorithm>
#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "PrimitiveGeometry.h"
#include "RenderPieplineList.h"

namespace {
	typedef struct
	{
		float distance;
		MyosotisFW::System::Render::StaticMesh_ptr staticMesh;
	}TransparentStaticMesh;	// Zソート用

	uint32_t g_shadowMapSize = 2048;

	float g_debugTimer = 0.0f;
	float g_debugSpeed = 50.0f;
}

namespace MyosotisFW::System::Render
{
	RenderSubsystem::RenderSubsystem(GLFWwindow& glfwWindow, VkInstance& instance, VkSurfaceKHR& surface)
	{
		m_instance = instance;
		m_onPressedSaveGameStageCallback = nullptr;

		// RenderDevice
		m_device = CreateRenderDevicePointer(m_instance);

		// Swapchain
		m_swapchain = CreateRenderSwapchainPointer(m_device, surface);

		// Resources
		m_resources = CreateRenderResourcesPointer(m_device);

		// depth/stencil
		prepareDepthStencil();

		// deferred rendering attachments
		prepareDeferredRendering();

		// render pass
		prepareRenderPass();

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

		// framebuffers
		prepareFrameBuffers();

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

		m_shadowMapRenderPipeline = CreateShadowMapRenderPipelinePointer(m_device, m_resources, m_renderPass.lighting.renderPass, m_shadowMap);
		m_deferredRenderPipeline = CreateDeferredRenderPipelinePointer(m_device, m_resources, m_renderPass.staticMesh.renderPass);
		m_compositionRenderPipeline = CreateCompositionRenderPipelinePointer(m_device, m_resources, m_renderPass.staticMesh.renderPass);
		m_compositionRenderPipeline->CreateShaderObject(m_compositionShaderBase, m_position, m_baseColor);
		m_transparentRenderPipeline = CreateTransparentRenderPipelinePointer(m_device, m_resources, m_renderPass.staticMesh.renderPass);
	}

	RenderSubsystem::~RenderSubsystem()
	{
		{// attachment
			vmaDestroyImage(m_device->GetVmaAllocator(), m_position.image, m_position.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_baseColor.image, m_baseColor.allocation);
			vkDestroyImageView(*m_device, m_position.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_baseColor.view, m_device->GetAllocationCallbacks());
		}

		{// furstum culler
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.frustumPlanesUBO.buffer.buffer, m_frustumCullerShaderObject.frustumPlanesUBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.objectDataSSBO.buffer.buffer, m_frustumCullerShaderObject.objectDataSSBO.buffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.buffer, m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocation);
			vkDestroyPipeline(*m_device, m_frustumCullerShaderObject.shaderBase.pipeline, m_device->GetAllocationCallbacks());
			vkDestroyPipelineLayout(*m_device, m_frustumCullerShaderObject.shaderBase.pipelineLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorSetLayout(*m_device, m_frustumCullerShaderObject.shaderBase.descriptorSetLayout, m_device->GetAllocationCallbacks());
			vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		}

		{// shadow map
			vmaDestroyImage(m_device->GetVmaAllocator(), m_shadowMap.image, m_shadowMap.allocation);
			vkDestroyImageView(*m_device, m_shadowMap.view, m_device->GetAllocationCallbacks());
		}

		vkDestroySemaphore(*m_device, m_semaphores.presentComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.computeComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.renderComplete, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& framebuffer : m_renderPass.staticMesh.framebuffer)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, m_device->GetAllocationCallbacks());
		}
		for (VkFramebuffer& framebuffer : m_renderPass.lighting.framebuffer)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, m_device->GetAllocationCallbacks());
		}
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		vkDestroyCommandPool(*m_device, m_renderCommandPool, m_device->GetAllocationCallbacks());
		vkDestroyCommandPool(*m_device, m_computeCommandPool, m_device->GetAllocationCallbacks());
		vkDestroyRenderPass(*m_device, m_renderPass.staticMesh.renderPass, m_device->GetAllocationCallbacks());
		vkDestroyRenderPass(*m_device, m_renderPass.lighting.renderPass, m_device->GetAllocationCallbacks());
		vkDestroyImage(*m_device, m_depthStencil.image, m_device->GetAllocationCallbacks());
		vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
		vkFreeMemory(*m_device, m_depthStencil.memory, m_device->GetAllocationCallbacks());
	}

	void RenderSubsystem::ResetMousePos(glm::vec2 mousePos)
	{
		if (!m_mainCamera) return;

		Object_Cast<Camera::FPSCamera>(m_mainCamera)->ResetMousePos(mousePos);
	}

	void RenderSubsystem::RegisterObject(ObjectBase_ptr& object)
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
			m_transparentRenderPipeline->CreateShaderObject(shaderObject);
			m_deferredRenderPipeline->CreateShaderObject(shaderObject);
		}
		break;
		default:
			break;
		}
		m_objects.push_back(object);
	}

	void RenderSubsystem::Update(UpdateData updateData)
	{
		if (m_mainCamera)
		{
			Object_Cast<Camera::FPSCamera>(m_mainCamera)->Update(updateData);
		}

		StaticMesh_ptr firstStaticMesh = nullptr;
		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);
				staticMesh->Update(updateData, m_mainCamera);
				if (!firstStaticMesh) firstStaticMesh = staticMesh;
			}
		}

		// TEST
		g_debugTimer += updateData.deltaTime * g_debugSpeed;
		if (g_debugTimer >= 360.0f)
		{
			g_debugTimer -= 360.0f;
		}
		firstStaticMesh->SetRot(glm::vec3(g_debugTimer, 0.0f, 0.0f));
		firstStaticMesh->Update(updateData, m_mainCamera);
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
	}

	void RenderSubsystem::MeshRender()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkBeginCommandBuffer(currentCommandBuffer, &commandBufferBeginInfo));

		{// ShadowMap Prender Pass
			// todo. Change to run when loading .gs
			m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.1f, 0.5f), "ShadowMap Prender Pass"));

			std::vector<VkClearValue> clearValues(1);
			clearValues[0] = AppInfo::g_depthClearValues;

			VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass.lighting.renderPass, g_shadowMapSize, g_shadowMapSize, clearValues);
			renderPassBeginInfo.framebuffer = m_renderPass.lighting.framebuffer[0];

			vkCmdBeginRenderPass(currentCommandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(g_shadowMapSize), static_cast<float>(g_shadowMapSize));
			vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(g_shadowMapSize, g_shadowMapSize);
			vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

			vkCmdSetDepthBias(
				currentCommandBuffer,
				0.5f,
				0.0f,
				1.0f);

			m_frustumCullerShaderObject.objectDataSSBO.data.objects.clear();
			for (ObjectBase_ptr& object : m_objects)
			{
				if (IsStaticMesh(object->GetObjectType()))
				{
					Object_CastToStaticMesh(object)->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::ShadowMap);
				}
			}

			vkCmdEndRenderPass(currentCommandBuffer);

			m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
		}

		{// Static Mesh Render Pass
			m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.8f, 0.1f), "Static Mesh Render Pass"));

			std::vector<VkClearValue> clearValues(4);
			clearValues[0] = AppInfo::g_colorClearValues;
			clearValues[1] = AppInfo::g_colorClearValues;
			clearValues[2] = AppInfo::g_colorClearValues;
			clearValues[3] = AppInfo::g_depthClearValues;

			VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass.staticMesh.renderPass, m_swapchain->GetWidth(), m_swapchain->GetHeight(), clearValues);
			renderPassBeginInfo.framebuffer = m_renderPass.staticMesh.framebuffer[m_currentBufferIndex];

			vkCmdBeginRenderPass(currentCommandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight()));
			vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_swapchain->GetWidth(), m_swapchain->GetHeight());
			vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

			memcpy(m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.data(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.size() * sizeof(uint32_t));

			std::vector<TransparentStaticMesh> transparentStaticMeshes{};

			uint32_t staticMeshCounter = 0;
			for (ObjectBase_ptr& object : m_objects)
			{
				if (IsStaticMesh(object->GetObjectType()))
				{
					if (m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices[staticMeshCounter] == 1)
					{
						StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);

						// Zソート(簡易)
						transparentStaticMeshes.push_back({ m_mainCamera->GetDistance(staticMesh->GetPos()), staticMesh });
					}
					staticMeshCounter++;
				}
			}

			// 距離ソート
			std::sort(transparentStaticMeshes.begin(), transparentStaticMeshes.end(),
				[](const TransparentStaticMesh& a, const TransparentStaticMesh& b) {
					return a.distance > b.distance;
				});

			{// Deferred Render Pass
				m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.2f, 0.1f), "Deferred SubPass"));
				for (TransparentStaticMesh& staticMeshesPair : transparentStaticMeshes)
				{
					staticMeshesPair.staticMesh->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::Deferred);
					Logger::Info(std::string("rending: ") + std::to_string(transparentStaticMeshes.size()));
				}

				m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
			}

			vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			{// Composition Render Pass
				m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.4f, 0.1f), "Composition SubPass"));

				vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionShaderBase.pipeline);
				vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionShaderBase.pipelineLayout, 0, 1, &m_compositionShaderBase.descriptorSet, 0, NULL);
				vkCmdDraw(currentCommandBuffer, 3, 1, 0, 0);

				m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
			}

			vkCmdNextSubpass(currentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			{// Transparent Render Pass
				m_vkCmdBeginDebugUtilsLabelEXT(currentCommandBuffer, &Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.1f, 0.6f, 0.1f), "Transparent SubPass"));

				//for (TransparentStaticMesh staticMeshesPair : transparentStaticMeshes)
				//{
				//	staticMeshesPair.staticMesh->BindCommandBuffer(currentCommandBuffer, RenderPipelineType::Transparent);
				//	Logger::Info(std::string("rending: ") + std::to_string(staticMeshCounter));
				//}

				m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
			}

			vkCmdEndRenderPass(currentCommandBuffer);
			m_vkCmdEndDebugUtilsLabelEXT(currentCommandBuffer);
		}
		VK_VALIDATION(vkEndCommandBuffer(currentCommandBuffer));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &currentCommandBuffer;
		VK_VALIDATION(vkQueueSubmit(m_graphicsQueue, 1, &m_submitInfo, VK_NULL_HANDLE));
	}

	void RenderSubsystem::EndRender()
	{
		m_swapchain->QueuePresent(m_graphicsQueue, m_currentBufferIndex, m_semaphores.renderComplete);
		VK_VALIDATION(vkQueueWaitIdle(m_graphicsQueue));
	}

	void RenderSubsystem::Resize(VkSurfaceKHR& surface, uint32_t width, uint32_t height)
	{
		// デバイスの処理を待つ
		vkDeviceWaitIdle(*m_device);

		// swapchain
		m_swapchain.reset();
		m_swapchain = CreateRenderSwapchainPointer(m_device, surface);

		// depth/stencil
		vkDestroyImage(*m_device, m_depthStencil.image, m_device->GetAllocationCallbacks());
		vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
		vkFreeMemory(*m_device, m_depthStencil.memory, m_device->GetAllocationCallbacks());
		prepareDepthStencil();

		// framebuffers
		for (VkFramebuffer& framebuffer : m_renderPass.staticMesh.framebuffer)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, m_device->GetAllocationCallbacks());
		}
		for (VkFramebuffer& framebuffer : m_renderPass.lighting.framebuffer)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, m_device->GetAllocationCallbacks());
		}
		prepareFrameBuffers();

		// command buffers
		vkFreeCommandBuffers(*m_device, m_renderCommandPool, static_cast<uint32_t>(m_renderCommandBuffers.size()), m_renderCommandBuffers.data());
		vkFreeCommandBuffers(*m_device, m_computeCommandPool, static_cast<uint32_t>(m_computeCommandBuffers.size()), m_computeCommandBuffers.data());
		prepareCommandBuffers();

		// fences
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		prepareFences();

		if (m_mainCamera)
		{
			m_mainCamera->UpdateScreenSize(glm::vec2(width, height));
		}

		vkDeviceWaitIdle(*m_device);
	}

	void RenderSubsystem::prepareDepthStencil()
	{
		{// StaticMesh pass
			// image
			VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthFormat, m_swapchain->GetWidth(), m_swapchain->GetHeight());
			VK_VALIDATION(vkCreateImage(*m_device, &imageCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.image));
			// allocate
			m_device->ImageMemoryAllocate(m_depthStencil);
			// image view
			VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_depthStencil.image, AppInfo::g_depthFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.view));
		}

		{// Lighting pass
			// image
			VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_shadowMapFormat, g_shadowMapSize, g_shadowMapSize);
			imageCreateInfoForDepthStencil.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfoForDepthStencil, &allocationCreateInfo, &m_shadowMap.image, &m_shadowMap.allocation, &m_shadowMap.allocationInfo);

			// image view
			VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_shadowMap.image, AppInfo::g_shadowMapFormat);
			imageViewCreateInfoForDepthStencil.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_shadowMap.view));
		}
	}

	void RenderSubsystem::prepareRenderPass()
	{
		{// StaticMesh Pass
			// attachments
			std::vector<VkAttachmentDescription> attachments = {
				Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_surfaceFormat.format),			// color
				Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_deferredPositionFormat),		// position
				Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_deferredBaseColorFormat),	// base color
				Utility::Vulkan::CreateInfo::attachmentDescriptionForDepthStencil(AppInfo::g_depthFormat),				// depth/stencil
			};

			std::vector<VkSubpassDescription> subpassDescriptions{};

			VkAttachmentReference depthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(3, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			// deferred
			std::vector<VkAttachmentReference> deferredColorAttachments = {
				Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),			// [0] color
				Utility::Vulkan::CreateInfo::attachmentReference(1, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),			// [1] position
				Utility::Vulkan::CreateInfo::attachmentReference(2, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),			// [2] base color
			};
			subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription(deferredColorAttachments, depthAttachmentReference));

			// [deferred]composition
			std::vector<VkAttachmentReference> compositionColorAttachments = { Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) };
			std::vector<VkAttachmentReference> compositionInputAttachments = {
				Utility::Vulkan::CreateInfo::attachmentReference(1, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),			// [1] position
				Utility::Vulkan::CreateInfo::attachmentReference(2, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),			// [2] base color
			};
			subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription(compositionColorAttachments, depthAttachmentReference, compositionInputAttachments));

			// transparent
			std::vector<VkAttachmentReference> transparentColorAttachments = { Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) };
			subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription(transparentColorAttachments, depthAttachmentReference));

			// dependency
			std::vector<VkSubpassDependency> dependencies = {
				// start -> deferred
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					0,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					0,
					VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
					0),
					// start -> deferred
					Utility::Vulkan::CreateInfo::subpassDependency(
						VK_SUBPASS_EXTERNAL,
						0,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						0,
						VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						0),
					// deferred -> composition
					Utility::Vulkan::CreateInfo::subpassDependency(
						0,
						1,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						VkAccessFlagBits::VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
						VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
					// composition -> transparent
					Utility::Vulkan::CreateInfo::subpassDependency(
						1,
						2,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						VkAccessFlagBits::VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
						VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
					// transparent -> end
					Utility::Vulkan::CreateInfo::subpassDependency(
						2,
						VK_SUBPASS_EXTERNAL,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT,
						VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
			};

			VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
			VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass.staticMesh.renderPass));
		}

		{// Lighting Pass
			// attachments
			std::vector<VkAttachmentDescription> attachments = {
				Utility::Vulkan::CreateInfo::attachmentDescriptionForShadowMap(AppInfo::g_shadowMapFormat),			// shadow map
			};

			std::vector<VkSubpassDescription> subpassDescriptions{};

			// deferred
			VkAttachmentReference depthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescriptionDepthStencilOnly(depthAttachmentReference));

			// dependency
			std::vector<VkSubpassDependency> dependencies = {
				// start -> deferred
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					0,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
					VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
					VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
					// start -> deferred
					Utility::Vulkan::CreateInfo::subpassDependency(
						0,
						VK_SUBPASS_EXTERNAL,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
						VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
			};

			VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
			VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass.lighting.renderPass));
		}
	}

	void RenderSubsystem::prepareFrameBuffers()
	{
		{// StaticMesh pass
			std::array<VkImageView, 4> attachments = {};

			// Depth/Stencil
			attachments[3] = m_depthStencil.view;

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = NULL;
			frameBufferCreateInfo.renderPass = m_renderPass.staticMesh.renderPass;
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = m_swapchain->GetWidth();
			frameBufferCreateInfo.height = m_swapchain->GetHeight();
			frameBufferCreateInfo.layers = 1;
			m_renderPass.staticMesh.framebuffer.resize(m_swapchain->GetImageCount());
			for (uint32_t i = 0; i < m_renderPass.staticMesh.framebuffer.size(); i++)
			{
				attachments[0] = m_swapchain->GetSwapchainImage()[i].view;
				attachments[1] = m_position.view;
				attachments[2] = m_baseColor.view;
				VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_renderPass.staticMesh.framebuffer[i]));
			}
		}

		{// Lighting pass
			// Depth/Stencil
			VkImageView attachment = m_shadowMap.view;

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = NULL;
			frameBufferCreateInfo.renderPass = m_renderPass.lighting.renderPass;
			frameBufferCreateInfo.attachmentCount = 1;
			frameBufferCreateInfo.pAttachments = &attachment;
			frameBufferCreateInfo.width = g_shadowMapSize;
			frameBufferCreateInfo.height = g_shadowMapSize;
			frameBufferCreateInfo.layers = 1;
			m_renderPass.lighting.framebuffer.resize(1);
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_renderPass.lighting.framebuffer[0]));
		}
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

	void RenderSubsystem::prepareDeferredRendering()
	{
		{// attachments position
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredPositionFormat, m_swapchain->GetWidth(), m_swapchain->GetHeight());
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_position.image, &m_position.allocation, &m_position.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_position.image, AppInfo::g_deferredPositionFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_position.view));
		}

		{// attachments base color
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredBaseColorFormat, m_swapchain->GetWidth(), m_swapchain->GetHeight());
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_baseColor.image, &m_baseColor.allocation, &m_baseColor.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_baseColor.image, AppInfo::g_deferredBaseColorFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_baseColor.view));
		}
	}

	void RenderSubsystem::prepareFrustumCuller()
	{
		{// frustumPlanesUBO
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_frustumCullerShaderObject.frustumPlanesUBO.data,
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

	void RenderSubsystem::SetOnPressedSaveGameStageCallback(OnPressedSaveGameStageCallback callback)
	{
		m_onPressedSaveGameStageCallback = callback;
	}
	void RenderSubsystem::SetOnPressedLoadGameStageCallback(OnPressedLoadGameStageCallback callback)
	{
		m_onPressedLoadGameStageCallback = callback;
	}
	void RenderSubsystem::SetOnPressedCreateObjectCallback(OnPressedCreateObjectCallback callback)
	{
		m_onPressedCreateObjectCallback = callback;
	}
}