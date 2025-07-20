// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "RenderDevice.h"
#include "RenderResources.h"
#include "RenderDescriptors.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	class RenderPipelineBase
	{
	public:
		RenderPipelineBase(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			m_device(device),
			m_pipelineLayout(VK_NULL_HANDLE),
			m_pipeline(VK_NULL_HANDLE),
			m_descriptors(descriptors) {
		}
		virtual ~RenderPipelineBase() = default;

		virtual void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;

	protected:
		virtual void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;

		RenderDevice_ptr m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		RenderDescriptors_ptr m_descriptors;
	};
}