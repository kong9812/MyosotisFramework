// Copyright (c) 2025 kong9812
#include "SkyboxRenderPass.h"
#include <vector>
#include <array>

#include "RenderSwapchain.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	SkyboxRenderPass::SkyboxRenderPass(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderSwapchain_ptr& swapchain) :
		RenderPassBase(device, resources, swapchain->GetScreenSize()) {
	}

	SkyboxRenderPass::~SkyboxRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void SkyboxRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForColor(AppInfo::g_surfaceFormat.format),// [0] main render target
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// skybox subpass
		VkAttachmentReference skyboxSubpassColorAttachmentReferences = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::MainRenderTarget), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color(skyboxSubpassColorAttachmentReferences));

		std::vector<VkSubpassDependency> dependencies = {
			// 外部 -> skybox (Color)
			// MainRenderTarget 最初の書き込み
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::Skybox),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkAccessFlagBits::VK_ACCESS_NONE,
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),

				// skybox -> 外部 (Color)
				// 次は Lighting FragmentShader の時に ColorAttachment として書き込み
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::Skybox),
					VK_SUBPASS_EXTERNAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT)
		};


		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		createFrameBuffers();
	}

	void SkyboxRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::MainRenderTarget)] = AppInfo::g_colorClearValues_white;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_screenSize.x, m_screenSize.y, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[frameIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_screenSize.x), static_cast<float>(m_screenSize.y));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_screenSize.x, m_screenSize.y);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void SkyboxRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void SkyboxRenderPass::createFrameBuffers()
	{
		std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
		m_framebuffers.resize(AppInfo::g_maxInFlightFrameCount);

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
		{
			attachments[static_cast<uint32_t>(Attachments::MainRenderTarget)] = m_resources->GetMainRenderTarget(i).view;

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