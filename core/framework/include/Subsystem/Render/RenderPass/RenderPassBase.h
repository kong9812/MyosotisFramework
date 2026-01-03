// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

#include "RenderDevice.h"
#include "RenderResources.h"
#include "iglm.h"

namespace MyosotisFW::System::Render
{
	class RenderPassBase
	{
	public:
		RenderPassBase(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const glm::ivec2& screenSize) :
			m_device(device),
			m_resources(resources),
			m_screenSize(screenSize),
			m_renderPass(VK_NULL_HANDLE),
			m_framebuffers() {
		}
		virtual ~RenderPassBase() = default;

		virtual void Initialize() = 0;

		VkRenderPass GetRenderPass() const { return m_renderPass; }

		virtual void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t currentBufferIndex) = 0;
		virtual void EndRender(const VkCommandBuffer& commandBuffer) = 0;
		virtual void Resize(const glm::ivec2& screenSize)
		{
			m_screenSize = screenSize;
			for (VkFramebuffer& m_framebuffer : m_framebuffers)
			{
				vkDestroyFramebuffer(*m_device, m_framebuffer, m_device->GetAllocationCallbacks());
			}
			createFrameBuffers();
		}

	protected:
		virtual void createFrameBuffers() = 0;

		RenderDevice_ptr m_device;
		RenderResources_ptr m_resources;

		glm::ivec2 m_screenSize;

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;
	};
}