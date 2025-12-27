// Copyright (c) 2025 kong9812
#include "VisibilityBufferRenderPass.h"
#include <vector>
#include <array>

#include "RenderSwapchain.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	VisibilityBufferRenderPass::VisibilityBufferRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
		RenderPassBase(device, resources, swapchain->GetWidth(), swapchain->GetHeight()) {
	}

	VisibilityBufferRenderPass::~VisibilityBufferRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void VisibilityBufferRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_visibilityBufferFormat),	// [0] main render target
			Utility::Vulkan::CreateInfo::attachmentDescriptionForDepth(AppInfo::g_depthBufferFormat),		// [1] Primary Depth
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// Phase1 subpass
		VkAttachmentReference phase1SubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::VisibilityBuffer), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference phase1SubpassDepthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::DepthBuffer), VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_depth(phase1SubpassColorAttachmentReferences, phase1SubpassDepthAttachmentReference));

		std::vector<VkSubpassDependency> dependencies = {
			// 外部 -> Phase1 (Color)
			// VisibilityBuffer 最初の書き込み
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::Phase1),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkAccessFlagBits::VK_ACCESS_NONE,
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),

				// 外部 -> Phase1 (Depth)
				// DepthBuffer クリアしてから最初の書き込み
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::Phase1),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VkAccessFlagBits::VK_ACCESS_NONE,
					VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),

				// Phase1 -> 外部 (Color)
				// 次は Lighting の時 Sampler 経由で参照する
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::Phase1),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT)
		};


		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		createFrameBuffers();
	}

	void VisibilityBufferRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::VisibilityBuffer)] = AppInfo::g_vbClearValues;
		clearValues[static_cast<uint32_t>(Attachments::DepthBuffer)] = AppInfo::g_depthClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[frameIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_width), static_cast<float>(m_height));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_width, m_height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VisibilityBufferRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void VisibilityBufferRenderPass::createFrameBuffers()
	{
		std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
		m_framebuffers.resize(AppInfo::g_maxInFlightFrameCount);

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
		{
			attachments[static_cast<uint32_t>(Attachments::VisibilityBuffer)] = m_resources->GetVisibilityBuffer(i).view;
			attachments[static_cast<uint32_t>(Attachments::DepthBuffer)] = m_resources->GetDepthBuffer(i).view;

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = NULL;
			frameBufferCreateInfo.renderPass = m_renderPass;
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments::COUNT);
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = m_width;
			frameBufferCreateInfo.height = m_height;
			frameBufferCreateInfo.layers = 1;
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_framebuffers[i]));
		}
	}
}