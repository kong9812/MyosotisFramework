// Copyright (c) 2025 kong9812
#include "subsystem/renderSubsystem.h"

#include <array>
#include "vkValidation.h"
#include "vkCreateInfo.h"
#include "appInfo.h"

namespace MyosotisFW::System::Render
{
	RenderSubsystem::RenderSubsystem(VkInstance& instance, VkSurfaceKHR& surface)
	{
		m_instance = instance;

		// RenderDevice
		m_device = std::make_shared<RenderDevice>(m_instance);
		
		// Swapchain
		m_swapchain = std::make_unique<RenderSwapchain>(m_device, surface);

		// Camera
		m_camera = std::make_shared<Camera::CameraBase>();

		// depth/stencil
		prepareDepthStencil();

		// render pass
		prepareRenderPass();

		// graphics queue
		vkGetDeviceQueue(*m_device, m_device->GetGraphicsFamilyIndex(), 0, &m_queue);

		// command pool
		VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetGraphicsFamilyIndex());
		VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, nullptr, &m_commandPool));
		prepareCommandBuffers();

		// fences
		prepareFences();

		// pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = Utility::Vulkan::CreateInfo::pipelineCacheCreateInfo();
		VK_VALIDATION(vkCreatePipelineCache(*m_device, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));

		// framebuffers
		prepareFrameBuffers();

		// semaphore(present/render)
		VkSemaphoreCreateInfo semaphoreCreateInfo = Utility::Vulkan::CreateInfo::semaphoreCreateInfo();
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.presentComplete));
		VK_VALIDATION(vkCreateSemaphore(*m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.renderComplete));

		// submit info
		m_submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		m_submitInfo = Utility::Vulkan::CreateInfo::submitInfo(m_submitPipelineStages, m_semaphores.presentComplete, m_semaphores.renderComplete);
	
		// bind command buffers
		buildCommandBuffers();
	}

	RenderSubsystem::~RenderSubsystem()
	{
		vkDestroySemaphore(*m_device, m_semaphores.presentComplete, nullptr);
		vkDestroySemaphore(*m_device, m_semaphores.renderComplete, nullptr);
		for (VkFramebuffer& framebuffer : m_frameBuffers)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
		}
		vkDestroyPipelineCache(*m_device, m_pipelineCache, nullptr);
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, nullptr);
		}
		vkFreeCommandBuffers(*m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		vkDestroyCommandPool(*m_device, m_commandPool, nullptr);
		vkDestroyRenderPass(*m_device, m_renderPass, nullptr);
		vkDestroyImage(*m_device, m_depthStencil.image, nullptr);
		vkDestroyImageView(*m_device, m_depthStencil.view, nullptr);
		vkFreeMemory(*m_device, m_depthStencil.memory, nullptr);
	}

	void RenderSubsystem::Render()
	{
		m_swapchain->AcquireNextImage(m_semaphores.presentComplete, m_currentBufferIndex);
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
		m_swapchain = std::make_unique<RenderSwapchain>(m_device, surface);

		// depth/stencil
		vkDestroyImage(*m_device, m_depthStencil.image, nullptr);
		vkDestroyImageView(*m_device, m_depthStencil.view, nullptr);
		vkFreeMemory(*m_device, m_depthStencil.memory, nullptr);
		prepareDepthStencil();

		// framebuffers
		for (VkFramebuffer& framebuffer : m_frameBuffers)
		{
			vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
		}
		prepareFrameBuffers();

		// command buffers
		vkFreeCommandBuffers(*m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		prepareCommandBuffers();
		buildCommandBuffers();

		// fences
		for (VkFence& fence : m_fences) {
			vkDestroyFence(*m_device, fence, nullptr);
		}
		prepareFences();

		vkDeviceWaitIdle(*m_device);
	}

	void RenderSubsystem::prepareDepthStencil()
	{
		// image
		VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthFormat, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		VK_VALIDATION(vkCreateImage(*m_device, &imageCreateInfoForDepthStencil, nullptr, &m_depthStencil.image));
		// allocate
		m_device->ImageMemoryAllocate(m_depthStencil);

		// image view
		VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_depthStencil.image, AppInfo::g_depthFormat);
		VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, nullptr, &m_depthStencil.view));
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
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, nullptr, &m_renderPass));
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
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
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
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, nullptr, &fence));
		}
	}

	void RenderSubsystem::buildCommandBuffers()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();

		std::vector<VkClearValue> clearValues = {
			AppInfo::g_colorClearValues,
			AppInfo::g_depthClearValues,
		};

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_swapchain->GetWidth(), m_swapchain->GetHeight(), clearValues);

		for (int32_t i = 0; i < m_commandBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = m_frameBuffers[i];

			VK_VALIDATION(vkBeginCommandBuffer(m_commandBuffers[i], &commandBufferBeginInfo));

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight()));
			vkCmdSetViewport(m_commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_swapchain->GetWidth(), m_swapchain->GetHeight());
			vkCmdSetScissor(m_commandBuffers[i], 0, 1, &scissor);

			// bind here


			vkCmdEndRenderPass(m_commandBuffers[i]);

			VK_VALIDATION(vkEndCommandBuffer(m_commandBuffers[i]));
		}
	}
}