// Copyright (c) 2025 kong9812
#include "subsystem/renderSubsystem.h"

#include <array>
#include <backends/imgui_impl_vulkan.h>

#include "vkValidation.h"
#include "vkCreateInfo.h"
#include "appInfo.h"
#ifdef DEBUG
#include "primitiveGeometry.h"
#endif

namespace MyosotisFW::System::Render
{
	RenderSubsystem::RenderSubsystem(GLFWwindow& glfwWindow, VkInstance& instance, VkSurfaceKHR& surface)
	{
		m_instance = instance;

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
		vkGetDeviceQueue(*m_device, m_device->GetGraphicsFamilyIndex(), 0, &m_queue);

		// command pool
		VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetGraphicsFamilyIndex());
		VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_commandPool));
		prepareCommandBuffers();

		// fences
		prepareFences();

		// pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = Utility::Vulkan::CreateInfo::pipelineCacheCreateInfo();
		VK_VALIDATION(vkCreatePipelineCache(*m_device, &pipelineCacheCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineCache));

		// framebuffers
		prepareFrameBuffers();

		// semaphore(present/render)
		VkSemaphoreCreateInfo semaphoreCreateInfo = Utility::Vulkan::CreateInfo::semaphoreCreateInfo();
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.presentComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, m_device->GetAllocationCallbacks(), &m_semaphores.renderComplete));

		// submit info
		m_submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		m_submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.renderComplete);
	
		// debug gui
#ifndef RELEASE
		m_debugGUI = CreateDebugGUIPointer(glfwWindow, m_instance, m_device, m_queue, m_renderPass, m_swapchain, m_pipelineCache);
#endif
		m_staticMeshes.push_back(CreatePrimitiveGeometryPointer(m_device, m_resources, m_renderPass, m_pipelineCache));

		// camera
		m_fpsCamera = Camera::CreateFPSCameraPointer();
	}

	RenderSubsystem::~RenderSubsystem()
	{
		vkDestroySemaphore(*m_device, m_semaphores.presentComplete, m_device->GetAllocationCallbacks());
		vkDestroySemaphore(*m_device, m_semaphores.renderComplete, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& framebuffer : m_frameBuffers)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, m_device->GetAllocationCallbacks());
		}
		vkDestroyPipelineCache(*m_device, m_pipelineCache, m_device->GetAllocationCallbacks());
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		vkFreeCommandBuffers(*m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		vkDestroyCommandPool(*m_device, m_commandPool, m_device->GetAllocationCallbacks());
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		vkDestroyImage(*m_device, m_depthStencil.image, m_device->GetAllocationCallbacks());
		vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
		vkFreeMemory(*m_device, m_depthStencil.memory, m_device->GetAllocationCallbacks());
	}

	void RenderSubsystem::Update()
	{
		for (StaticMesh_ptr& staticMesh : m_staticMeshes)
		{
			staticMesh->Update(*m_fpsCamera);
		}
	}

	void RenderSubsystem::Render()
	{
		m_swapchain->AcquireNextImage(m_semaphores.presentComplete, m_currentBufferIndex);
		buildCommandBuffer(m_currentBufferIndex);

		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &m_commandBuffers[m_currentBufferIndex];
		VK_VALIDATION(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));
		m_swapchain->QueuePresent(m_queue, m_currentBufferIndex, m_semaphores.renderComplete);
		VK_VALIDATION(vkQueueWaitIdle(m_queue));
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
		vkFreeCommandBuffers(*m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		prepareCommandBuffers();

		// fences
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
		prepareFences();

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
		// command buffers
		m_commandBuffers.resize(m_swapchain->GetImageCount());
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_commandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_swapchain->GetImageCount()));
		VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, m_commandBuffers.data()));
	}

	void RenderSubsystem::prepareFences()
	{
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo(VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT);
		m_fences.resize(m_commandBuffers.size());
		for (VkFence& fence : m_fences) {
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));
		}
	}

	void RenderSubsystem::buildCommandBuffer(uint32_t bufferIndex)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();

		std::vector<VkClearValue> clearValues = {
			AppInfo::g_colorClearValues,
			AppInfo::g_depthClearValues,
		};

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_swapchain->GetWidth(), m_swapchain->GetHeight(), clearValues);

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = m_frameBuffers[bufferIndex];

		VK_VALIDATION(vkBeginCommandBuffer(m_commandBuffers[bufferIndex], &commandBufferBeginInfo));

		vkCmdBeginRenderPass(m_commandBuffers[bufferIndex], &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight()));
		vkCmdSetViewport(m_commandBuffers[bufferIndex], 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_swapchain->GetWidth(), m_swapchain->GetHeight());
		vkCmdSetScissor(m_commandBuffers[bufferIndex], 0, 1, &scissor);

		// bind here
		for (StaticMesh_ptr& staticMesh : m_staticMeshes)
		{
			staticMesh->BindCommandBuffer(m_commandBuffers[bufferIndex]);
		}

#ifndef RELEASE
		m_debugGUI->BuildCommandBuffer(m_commandBuffers[bufferIndex]);
#endif // !RELEASE

		vkCmdEndRenderPass(m_commandBuffers[bufferIndex]);

		VK_VALIDATION(vkEndCommandBuffer(m_commandBuffers[bufferIndex]));
	}
}