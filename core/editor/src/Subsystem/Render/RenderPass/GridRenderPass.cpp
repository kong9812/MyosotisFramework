// Copyright (c) 2025 kong9812
#include "GridRenderPass.h"
#include <vector>
#include <array>

#include "RenderSwapchain.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	GridRenderPass::GridRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
		RenderPassBase(device, resources, swapchain->GetScreenSize()) {
	}

	GridRenderPass::~GridRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void GridRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_surfaceFormat.format,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),		// [0] main render target (次はRenderSubsystemでSwapchainImageにコピー)

			Utility::Vulkan::CreateInfo::attachmentDescriptionForDepth(AppInfo::g_depthBufferFormat,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),	// [1] Primary Depth
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// render subpass
		VkAttachmentReference renderSubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::MainRenderTarget), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference renderSubpassDepthAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::DepthBuffer), VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_depth(renderSubpassColorAttachmentReferences, renderSubpassDepthAttachmentReferences));

		std::vector<VkSubpassDependency> dependencies = {
			// 外部 -> Render (Depth)
			// Depthはphase2が最後に使った (参照するが書き出さない)
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::Render),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
				VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),

				// 外部 -> Render (Color)
				// Lighting後の書き込む
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::Render),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),

				// Render -> 外部 (Color)
				// SwapchainImageにコピーの準備
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Render),
					VK_SUBPASS_EXTERNAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT)
		};


		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		createFrameBuffers();
	}

	void GridRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::MainRenderTarget)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::DepthBuffer)] = AppInfo::g_depthClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_screenSize.x, m_screenSize.y, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[frameIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_screenSize.x), static_cast<float>(m_screenSize.y));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_screenSize.x, m_screenSize.y);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void GridRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void GridRenderPass::createFrameBuffers()
	{
		std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
		m_framebuffers.resize(AppInfo::g_maxInFlightFrameCount);

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
		{
			attachments[static_cast<uint32_t>(Attachments::MainRenderTarget)] = m_resources->GetMainRenderTarget(i).view;
			attachments[static_cast<uint32_t>(Attachments::DepthBuffer)] = m_resources->GetDepthBuffer(i).view;

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = NULL;
			frameBufferCreateInfo.renderPass = m_renderPass;
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments::COUNT);
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = m_screenSize.x;
			frameBufferCreateInfo.height = m_screenSize.y;
			frameBufferCreateInfo.layers = 1;
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_framebuffers[i]));
		}
	}
}