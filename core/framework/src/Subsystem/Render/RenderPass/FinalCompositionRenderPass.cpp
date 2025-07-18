// Copyright (c) 2025 kong9812
#include "FinalCompositionRenderPass.h"
#include <vector>
#include <array>

#include "RenderSwapchain.h"

#include "AppInfo.h"
#include "VK_Loader.h"

namespace MyosotisFW::System::Render
{
	FinalCompositionRenderPass::FinalCompositionRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
		RenderPassBase(device, resources, swapchain->GetWidth(), swapchain->GetHeight()),
		m_swapchain(swapchain) {
	}

	FinalCompositionRenderPass::~FinalCompositionRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void FinalCompositionRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_surfaceFormat.format),			// swapchain images
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_surfaceFormat.format,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),			// main render target
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// Composition subpass
		VkAttachmentReference compositionSubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::SwapchainImages), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference compositionSubpassInputAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::MainRenderTarget), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_input(compositionSubpassColorAttachmentReferences, compositionSubpassInputAttachmentReferences));

		std::vector<VkSubpassDependency> dependencies = {
			// start -> Composition subpass
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::Composition),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				0,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
				0),
				// start -> Composition subpass
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::Composition),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					0,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0),
				// Composition subpass -> end
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Composition),
					VK_SUBPASS_EXTERNAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
		};

		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		createFrameBuffers();
	}

	void FinalCompositionRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::SwapchainImages)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::MainRenderTarget)] = AppInfo::g_colorClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[currentBufferIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_width), static_cast<float>(m_height));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_width, m_height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void FinalCompositionRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void FinalCompositionRenderPass::createFrameBuffers()
	{
		std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
		attachments[static_cast<uint32_t>(Attachments::MainRenderTarget)] = m_resources->GetMainRenderTarget().view;

		m_framebuffers.resize(m_swapchain->GetImageCount());
		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments::COUNT);
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = m_swapchain->GetWidth();
		frameBufferCreateInfo.height = m_swapchain->GetHeight();
		frameBufferCreateInfo.layers = 1;
		for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		{
			attachments[static_cast<uint32_t>(Attachments::SwapchainImages)] = m_swapchain->GetSwapchainImage()[i].view;
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_framebuffers[i]));
		}
	}
}