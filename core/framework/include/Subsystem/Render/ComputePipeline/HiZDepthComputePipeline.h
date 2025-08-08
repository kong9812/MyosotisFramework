// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

#include "Structs.h"
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderDescriptors;
	TYPEDEF_SHARED_PTR_FWD(RenderDescriptors);
	class RenderResources;
	TYPEDEF_SHARED_PTR_FWD(RenderResources);

	class HiZDepthComputePipeline
	{
	public:
		HiZDepthComputePipeline(const RenderDevice_ptr& device,
			const RenderDescriptors_ptr& descriptors,
			const RenderResources_ptr& resources) :
			m_device(device),
			m_descriptors(descriptors),
			m_resources(resources),
			depthDownsamplePushConstant({}),
			depthCopyPushConstant({}),
			m_hiZDepthCopyShaderBase({}),
			m_hiZDepthDownsampleShaderBase({}) {
		}
		~HiZDepthComputePipeline();

		void Initialize();
		void Dispatch(const VkCommandBuffer& commandBuffer, const glm::vec2& screenSize);

	private:
		struct {
			glm::ivec2 desSize;
			uint32_t hiZImageID;
			uint32_t primaryDepthSamplerID;
		}depthCopyPushConstant;

		struct {
			glm::ivec2 desSize;
			uint32_t hiZImageID;
			uint32_t hiZSamplerID;
			int32_t srcMip;
		}depthDownsamplePushConstant;

		RenderDevice_ptr m_device;
		RenderDescriptors_ptr m_descriptors;
		RenderResources_ptr m_resources;

		ShaderBase m_hiZDepthCopyShaderBase;
		ShaderBase m_hiZDepthDownsampleShaderBase;
	};
	TYPEDEF_SHARED_PTR_ARGS(HiZDepthComputePipeline);
}