// Copyright (c) 2025 kong9812
#include "MainRenderPass.h"
#include <vector>
#include <array>

#include "AppInfo.h"
#include "VK_Loader.h"

namespace MyosotisFW::System::Render
{
	MainRenderPass::~MainRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void MainRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			// [0] main render target
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_surfaceFormat.format,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
				VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),

			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_surfaceFormat.format),		// [1] lighting result image
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_deferredPositionFormat),		// [2] [g-buffer] position

			// [3] [g-buffer] normal + ID
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_deferredNormalFormat,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),

			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_colorFormat),				// [4] [g-buffer] base color
			Utility::Vulkan::CreateInfo::attachmentDescriptionForDepthStencil(AppInfo::g_depthFormat),				// [5] depth/stencil
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// G-Buffer fill subpass
		std::vector<VkAttachmentReference> gBufferFillSubpassColorAttachmentReferences = {
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferPosition), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferNormal), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferBaseColor), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
		};
		VkAttachmentReference gBufferFillSubpassDepthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::DepthStencil), VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_colors_depth(gBufferFillSubpassColorAttachmentReferences, gBufferFillSubpassDepthAttachmentReference));

		// Lighting subpass
		VkAttachmentReference lightingSubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::LightingResultImage), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		std::vector<VkAttachmentReference> lightingInputAttachmentReferences = {
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferPosition), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferNormal), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::GBufferBaseColor), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		};
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_inputs(lightingSubpassColorAttachmentReferences, lightingInputAttachmentReferences));

		// Composition subpass
		VkAttachmentReference compositionSubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::MainRenderTarget), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference compositionSubpassInputAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::LightingResultImage), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color_input(compositionSubpassColorAttachmentReferences, compositionSubpassInputAttachmentReferences));

		std::vector<VkSubpassDependency> dependencies = {
			// start -> G-Buffer fill subpass
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::GBufferFill),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				0,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
				0),
				// start -> G-Buffer fill subpass
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::GBufferFill),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					0,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0),
				// G-Buffer fill subpass -> Lighting subpass
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::GBufferFill),
					static_cast<uint32_t>(SubPass::Lighting),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
				// Lighting subpass -> Composition subpass
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Lighting),
					static_cast<uint32_t>(SubPass::Composition),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
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

	void MainRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::MainRenderTarget)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::LightingResultImage)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::GBufferPosition)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::GBufferNormal)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::GBufferBaseColor)] = AppInfo::g_colorClearValues;
		clearValues[static_cast<uint32_t>(Attachments::DepthStencil)] = AppInfo::g_depthClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[0];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_width), static_cast<float>(m_height));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_width, m_height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void MainRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void MainRenderPass::createFrameBuffers()
	{
		std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
		m_framebuffers.resize(1);

		attachments[static_cast<uint32_t>(Attachments::MainRenderTarget)] = m_resources->GetMainRenderTarget().view;
		attachments[static_cast<uint32_t>(Attachments::LightingResultImage)] = m_resources->GetLightingResult().view;
		attachments[static_cast<uint32_t>(Attachments::GBufferPosition)] = m_resources->GetPosition().view;
		attachments[static_cast<uint32_t>(Attachments::GBufferNormal)] = m_resources->GetNormal().view;
		attachments[static_cast<uint32_t>(Attachments::GBufferBaseColor)] = m_resources->GetBaseColor().view;
		attachments[static_cast<uint32_t>(Attachments::DepthStencil)] = m_resources->GetDepthStencil().view;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments::COUNT);
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = m_width;
		frameBufferCreateInfo.height = m_height;
		frameBufferCreateInfo.layers = 1;
		VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_framebuffers[0]));
	}
}