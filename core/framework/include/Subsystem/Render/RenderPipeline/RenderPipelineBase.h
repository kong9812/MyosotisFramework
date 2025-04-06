// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "RenderDevice.h"
#include "RenderResources.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	class RenderPipelineBase
	{
	public:
		RenderPipelineBase(const RenderDevice_ptr& device, const uint32_t& descriptorCount = AppInfo::g_descriptorCount) :
			m_device(device),
			m_descriptorPool(VK_NULL_HANDLE),
			m_descriptorSetLayout(VK_NULL_HANDLE),
			m_pipelineLayout(VK_NULL_HANDLE),
			m_pipeline(VK_NULL_HANDLE),
			m_descriptorCount(descriptorCount) {
		}
		virtual ~RenderPipelineBase() = default;

		virtual void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;
	protected:
		virtual void prepareDescriptors() = 0;
		virtual void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;

		RenderDevice_ptr m_device;

		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_descriptorSetLayout;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		uint32_t m_descriptorCount;
	};
}