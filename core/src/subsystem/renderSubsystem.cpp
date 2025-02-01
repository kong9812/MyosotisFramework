// Copyright (c) 2025 kong9812
#include "RenderSubsystem.h"

#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "PrimitiveGeometry.h"

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
		for (VkFramebuffer& framebuffer : m_frameBuffers)
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
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
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
			Object_CastToStaticMesh(object)->PrepareForRender(m_device, m_resources, m_renderPass);
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

		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				Object_CastToStaticMesh(object)->Update(updateData, m_mainCamera);
			}
		}
	}

	void RenderSubsystem::Compute()
	{
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.computeComplete);
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(m_computeCommandBuffers[0], &commandBufferBeginInfo));
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

	void RenderSubsystem::TransparentRender()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();

		std::vector<VkClearValue> clearValues = {
			AppInfo::g_colorClearValues,
			AppInfo::g_depthClearValues,
		};

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_swapchain->GetWidth(), m_swapchain->GetHeight(), clearValues);
		renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentBufferIndex];

		VK_VALIDATION(vkBeginCommandBuffer(m_renderCommandBuffers[m_currentBufferIndex], &commandBufferBeginInfo));

		vkCmdBeginRenderPass(m_renderCommandBuffers[m_currentBufferIndex], &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight()));
		vkCmdSetViewport(m_renderCommandBuffers[m_currentBufferIndex], 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_swapchain->GetWidth(), m_swapchain->GetHeight());
		vkCmdSetScissor(m_renderCommandBuffers[m_currentBufferIndex], 0, 1, &scissor);

		memcpy(m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.data(), m_frustumCullerShaderObject.visibleObjectsSSBO.buffer.allocationInfo.pMappedData, m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices.size() * sizeof(uint32_t));

		uint32_t staticMeshCounter = 0;
		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				if (m_frustumCullerShaderObject.visibleObjectsSSBO.data.visibleIndices[staticMeshCounter] == 1)
				{
					StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);
					staticMesh->BindCommandBuffer(m_renderCommandBuffers[m_currentBufferIndex]);
					Logger::Info(std::string("rending: ") + std::to_string(staticMeshCounter));
				}
				staticMeshCounter++;
			}
		}
		vkCmdEndRenderPass(m_renderCommandBuffers[m_currentBufferIndex]);
		VK_VALIDATION(vkEndCommandBuffer(m_renderCommandBuffers[m_currentBufferIndex]));
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &m_renderCommandBuffers[m_currentBufferIndex];
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
		for (VkFramebuffer& framebuffer : m_frameBuffers)
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
		// image
		VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthFormat, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		VK_VALIDATION(vkCreateImage(*m_device, &imageCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.image));
		// allocate
		m_device->ImageMemoryAllocate(m_depthStencil);

		// image view
		VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_depthStencil.image, AppInfo::g_depthFormat);
		VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.view));
	}

	void RenderSubsystem::prepareRenderPass()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_surfaceFormat.format),	// color
			Utility::Vulkan::CreateInfo::attachmentDescriptionForDepthStencil(AppInfo::g_depthFormat),		// depth/stencil
		};

		// subpass
		VkAttachmentReference colorReference = Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference depthReference = Utility::Vulkan::CreateInfo::attachmentReference(1, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		std::vector<VkSubpassDescription> subpassDescription = {
			Utility::Vulkan::CreateInfo::subpassDescription(colorReference, depthReference),
		};

		// dependency
		std::vector<VkSubpassDependency> dependencies = {
			Utility::Vulkan::CreateInfo::subpassDependencyForColor(),
			Utility::Vulkan::CreateInfo::subpassDependencyForDepthStencil(),
		};

		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescription);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));
	}

	void RenderSubsystem::prepareFrameBuffers()
	{
		std::array<VkImageView, 2> attachments = {};

		// Depth/Stencil
		attachments[1] = m_depthStencil.view;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = m_swapchain->GetWidth();
		frameBufferCreateInfo.height = m_swapchain->GetHeight();
		frameBufferCreateInfo.layers = 1;
		m_frameBuffers.resize(m_swapchain->GetImageCount());
		for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
		{
			attachments[0] = m_swapchain->GetSwapchainImage()[i].view;
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_frameBuffers[i]));
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

	void RenderSubsystem::prepareFrustumCuller()
	{
		{// frustumPlanesUBO
			vmaTools::ShaderBufferAllocate(
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
			vmaTools::ShaderBufferAllocate(
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
			vmaTools::ShaderBufferAllocate(
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