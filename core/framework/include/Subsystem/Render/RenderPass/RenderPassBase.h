// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

#include "RenderDevice.h"
#include "RenderResources.h"

namespace MyosotisFW::System::Render
{
	class RenderPassBase
	{
	public:
		RenderPassBase(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const uint32_t& width, const uint32_t& height) :
			m_device(device),
			m_resources(resources),
			m_width(width),
			m_height(height),
			m_renderPass(VK_NULL_HANDLE),
			m_framebuffers{} {
		};
		virtual ~RenderPassBase() = default;

		virtual void Initialize() = 0;

		VkRenderPass GetRenderPass() { return m_renderPass; }

		virtual void BeginRender(const VkCommandBuffer& commandBuffer, const uint32_t& currentBufferIndex) = 0;
		virtual void EndRender(const VkCommandBuffer& commandBuffer) = 0;

	protected:
		RenderDevice_ptr m_device;
		RenderResources_ptr m_resources;

		uint32_t m_width;
		uint32_t m_height;

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;
	};
}