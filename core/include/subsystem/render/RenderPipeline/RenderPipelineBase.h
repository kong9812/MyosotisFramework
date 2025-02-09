// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "RenderDevice.h"
#include "RenderResources.h"

namespace MyosotisFW::System::Render
{
	class RenderPipelineBase
	{
	public:
		virtual ~RenderPipelineBase() = default;

	protected:
		virtual void prepareDescriptors() = 0;
		virtual void prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass) = 0;

		RenderDevice_ptr m_device;

		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_descriptorSetLayout;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		uint32_t m_descriptorCount;
	};
}