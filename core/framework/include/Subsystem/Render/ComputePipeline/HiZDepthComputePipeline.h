// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

#include "Structs.h"
#include "ClassPointer.h"
#include "AppInfo.h"

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
		HiZDepthComputePipeline(const RenderDevice_ptr& device,
			const RenderResources_ptr& resources,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			m_device(device),
			m_resources(resources),
			depthDownsamplePushConstant({}),
			depthCopyPushConstant({}),
			m_hiZDepthCopyShaderBase({}),
			m_hiZDepthDownsampleShaderBase({}),
			m_sceneInfoDescriptorSet(sceneInfoDescriptorSet),
			m_objectInfoDescriptorSet(objectInfoDescriptorSet),
			m_meshInfoDescriptorSet(meshInfoDescriptorSet),
			m_textureDescriptorSet(textureDescriptorSet)
		{
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
		RenderResources_ptr m_resources;

		ShaderBase m_hiZDepthCopyShaderBase;
		ShaderBase m_hiZDepthDownsampleShaderBase;

		uint32_t m_hiZDepthMipMapImageIndex[AppInfo::g_hiZMipLevels];
		uint32_t m_hiZDepthMapMipSamplerIndex;

		SceneInfoDescriptorSet_ptr m_sceneInfoDescriptorSet;
		ObjectInfoDescriptorSet_ptr m_objectInfoDescriptorSet;
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;
		TextureDescriptorSet_ptr m_textureDescriptorSet;
	};
	TYPEDEF_SHARED_PTR_ARGS(HiZDepthComputePipeline);
}