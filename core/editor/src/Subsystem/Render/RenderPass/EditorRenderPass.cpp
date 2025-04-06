// Copyright (c) 2025 kong9812
#include "EditorRenderPass.h"
#include <vector>
#include <array>

#include "AppInfo.h"
#include "VK_Loader.h"

#include "EditorRenderResources.h"

namespace MyosotisFW::System::Render
{
	EditorRenderPass::~EditorRenderPass()
	{
		vkDestroyRenderPass(*m_device, m_renderPass, m_device->GetAllocationCallbacks());
		for (VkFramebuffer& m_framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
		}
	}

	void EditorRenderPass::Initialize()
	{
		// attachments
		std::vector<VkAttachmentDescription> attachments = {
			Utility::Vulkan::CreateInfo::attachmentDescriptionForAttachment(AppInfo::g_surfaceFormat.format,
				VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
				VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		};

		std::vector<VkSubpassDescription> subpassDescriptions{};

		// Editor GUI subpass
		VkAttachmentReference editorGUIColorAttachmentReference = Utility::Vulkan::CreateInfo::attachmentReference(static_cast<uint32_t>(Attachments::EditorRenderTarget), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		subpassDescriptions.push_back(Utility::Vulkan::CreateInfo::subpassDescription_color(editorGUIColorAttachmentReference));

		std::vector<VkSubpassDependency> dependencies = {
			// start -> EditorGUI subpass
			Utility::Vulkan::CreateInfo::subpassDependency(
				VK_SUBPASS_EXTERNAL,
				static_cast<uint32_t>(SubPass::EditorGUI),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				0,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
				0),
				// start -> EditorGUI subpass
				Utility::Vulkan::CreateInfo::subpassDependency(
					VK_SUBPASS_EXTERNAL,
					static_cast<uint32_t>(SubPass::EditorGUI),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					0,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0),
				// EditorGUI subpass -> end
				Utility::Vulkan::CreateInfo::subpassDependency(
					static_cast<uint32_t>(SubPass::EditorGUI),
					VK_SUBPASS_EXTERNAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT,
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT),
		};

		VkRenderPassCreateInfo renderPassInfo = Utility::Vulkan::CreateInfo::renderPassCreateInfo(attachments, dependencies, subpassDescriptions);
		VK_VALIDATION(vkCreateRenderPass(*m_device, &renderPassInfo, m_device->GetAllocationCallbacks(), &m_renderPass));

		{// StaticMesh pass
			std::array<VkImageView, static_cast<uint32_t>(Attachments::COUNT)> attachments{};
			m_framebuffers.resize(1);

			attachments[static_cast<uint32_t>(Attachments::EditorRenderTarget)] = std::dynamic_pointer_cast<EditorRenderResources>(m_resources)->GetEditorRenderTarget().view;

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

	void EditorRenderPass::BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex)
	{
		std::vector<VkClearValue> clearValues(static_cast<uint32_t>(Attachments::COUNT));
		clearValues[static_cast<uint32_t>(Attachments::EditorRenderTarget)] = AppInfo::g_colorClearValues;

		VkRenderPassBeginInfo renderPassBeginInfo = Utility::Vulkan::CreateInfo::renderPassBeginInfo(m_renderPass, m_width, m_height, clearValues);
		renderPassBeginInfo.framebuffer = m_framebuffers[0];

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = Utility::Vulkan::CreateInfo::viewport(static_cast<float>(m_width), static_cast<float>(m_height));
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = Utility::Vulkan::CreateInfo::rect2D(m_width, m_height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void EditorRenderPass::EndRender(const VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}
}