// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <vector>

#include "ShaderBase.h"
#include "ClassPointer.h"
#include "AppInfo.h"

#include "RenderDescriptors.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderResources;
	TYPEDEF_SHARED_PTR_FWD(RenderResources);
	class SceneInfoDescriptorSet;
	TYPEDEF_SHARED_PTR_FWD(SceneInfoDescriptorSet);
	class ObjectInfoDescriptorSet;
	TYPEDEF_SHARED_PTR_FWD(ObjectInfoDescriptorSet);
	class MeshInfoDescriptorSet;
	TYPEDEF_SHARED_PTR_FWD(MeshInfoDescriptorSet);
	class TextureDescriptorSet;
	TYPEDEF_SHARED_PTR_FWD(TextureDescriptorSet);

	class HiZDepthComputePipeline
	{
	public:
		HiZDepthComputePipeline(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const RenderDescriptors_ptr& renderDescriptors) :
			m_device(device),
			m_resources(resources),
			depthDownsamplePushConstant(),
			depthCopyPushConstant(),
			m_hiZDepthCopyShaderBase(),
			m_hiZDepthDownsampleShaderBase(),
			m_renderDescriptors(renderDescriptors),
			m_depthBufferSamplerID(),
			m_hiZDepthMipMapImageIndex() {
		}
		~HiZDepthComputePipeline();

		void Initialize();
		void Dispatch(const VkCommandBuffer& commandBuffer, const uint32_t dstFrameIndex, const uint32_t srcFrameIndex, const glm::vec2& screenSize);
		void Resize();

	private:
		struct DepthCopyPushConstant {
			glm::ivec2 desSize;
			uint32_t hiZImageID;
			uint32_t depthBufferSamplerID;
			uint32_t depthClear;
		}depthCopyPushConstant[AppInfo::g_maxInFlightFrameCount];

		struct DepthDownsamplePushConstant {
			glm::ivec2 desSize;
			uint32_t hiZImageID;
			uint32_t hiZSamplerID;
			int32_t srcMip;
		}depthDownsamplePushConstant[AppInfo::g_maxInFlightFrameCount];

		uint32_t m_depthBufferSamplerID[AppInfo::g_maxInFlightFrameCount];

		RenderDevice_ptr m_device;
		RenderResources_ptr m_resources;

		ShaderBase m_hiZDepthCopyShaderBase;
		ShaderBase m_hiZDepthDownsampleShaderBase;

		std::array<std::vector<uint32_t>, AppInfo::g_maxInFlightFrameCount> m_hiZDepthMipMapImageIndex;

		RenderDescriptors_ptr m_renderDescriptors;
	};
	TYPEDEF_SHARED_PTR_ARGS(HiZDepthComputePipeline);
}