// Copyright (c) 2025 kong9812
#include "ShadowMapRenderPass.h"
#include <vector>
#include <array>

#include "AppInfo.h"
#include "VK_Loader.h"

namespace MyosotisFW::System::Render
{
	ShadowMapRenderPass::~ShadowMapRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		vkDestroyFramebuffer(*m_device, m_framebuffers[0], m_device->GetAllocationCallbacks());
	}

	void ShadowMapRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForShadowMap(AppInfo::g_shadowMapFormat),	// shadow map
		};

		VkAttachmentReference depthAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(0, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkSubpassDescription subpassDescriptions = Utility::Vulkan::CreateInfo::subpassDescription_depth(depthAttachmentReference);

		// start -> deferred
		std::vector<VkSubpassDependency> dependencies =
		{
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
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		{// StaticMesh pass
			m_framebuffers.resize(1);

			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = NULL;
			frameBufferCreateInfo.renderPass = m_renderPass;
			frameBufferCreateInfo.attachmentCount = 1;
			frameBufferCreateInfo.pAttachments = &m_resources->GetShadowMap().view;
			frameBufferCreateInfo.width = AppInfo::g_shadowMapSize;
			frameBufferCreateInfo.height = AppInfo::g_shadowMapSize;
			frameBufferCreateInfo.layers = 1;
			VK_VALIDATION(vkCreateFramebuffer(*m_device, &frameBufferCreateInfo, m_device->GetAllocationCallbacks(), &m_framebuffers[0]));
		}
	}

	void ShadowMapRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex)
	{
		std::vector<VkClearValue> clearValues(1);
		clearValues[0] = AppInfo::g_depthClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[0];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_width), static_cast<float>(m_height));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_width, m_height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdSetDepthBias(commandBuffer, 2.25f, 0.0f, 2.75f);
	}

	void ShadowMapRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
		//m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
	}
}