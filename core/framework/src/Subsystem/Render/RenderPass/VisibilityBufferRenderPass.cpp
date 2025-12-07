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
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_visibilityBufferFormat,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE),// [0] main render target
			Utility::Vulkan::CreateInfo::attachmentDescriptionForDepth(AppInfo::g_primaryDepthFormat,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE),	// [1] Primary Depth
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// Phase1 subpass
		VkAttachmentReference phase1SubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::VisibilityBuffer), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference phase1SubpassDepthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::PrimaryDepth), VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_depth(phase1SubpassColorAttachmentReferences, phase1SubpassDepthAttachmentReference));

		// Phase2 subpass
		VkAttachmentReference phase2SubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::VisibilityBuffer), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference phase2SubpassDepthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::PrimaryDepth), VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_depth(phase2SubpassColorAttachmentReferences, phase2SubpassDepthAttachmentReference));

		std::vector<VkSubpassDependency> dependencies = {
			// 外部 -> Phase1（Depth/Stencil 初期化）
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::Phase1),
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				0,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				0
			),

				// 外部 -> Phase1（ColorAttachment 初期化）
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::Phase1),
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					0,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0
				),

				// Phase1 -> Phase2（Depth + Color をまとめて同期）
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Phase1),
					static_cast<uint32_t>(SubPass::Phase2),
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,       // srcStageMask
					VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT |          // dstStageMask
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,   // srcAccessMask
					VK_ACCESS_SHADER_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,    // dstAccessMask
					0 // BY_REGION を外す
				),

				// Phase2 -> 外部（完了待ち）
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Phase2),
					VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					0
				)
		};


		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		createFrameBuffers();
	}

	void VisibilityBufferRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t currentBufferIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::VisibilityBuffer)] = AppInfo::g_vbClearValues;
		clearValues[static_cast<uint32_t>(Attachments::PrimaryDepth)] = AppInfo::g_depthClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[0];

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
		m_framebuffers.resize(1);

		attachments[static_cast<uint32_t>(Attachments::VisibilityBuffer)] = m_resources->GetVisibilityBuffer().view;
		attachments[static_cast<uint32_t>(Attachments::PrimaryDepth)] = m_resources->GetPrimaryDepthStencil().view;

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
		{
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