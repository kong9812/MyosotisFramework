// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "RenderDevice.h"
#include "RenderResources.h"
#include "SceneInfoDescriptorSet.h"
#include "ObjectInfoDescriptorSet.h"
#include "MeshInfoDescriptorSet.h"
#include "TextureDescriptorSet.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	class RenderPipelineBase
	{
	public:
		RenderPipelineBase(const RenderDevice_ptr& device,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			m_device(device),
			m_pipelineLayout(VK_NULL_HANDLE),
			m_pipeline(VK_NULL_HANDLE),
			m_sceneInfoDescriptorSet(sceneInfoDescriptorSet),
			m_objectInfoDescriptorSet(objectInfoDescriptorSet),
			m_meshInfoDescriptorSet(meshInfoDescriptorSet),
			m_textureDescriptorSet(textureDescriptorSet)
		{
		}
		virtual ~RenderPipelineBase() = default;

		virtual void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;

	protected:
		virtual void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) = 0;

		RenderDevice_ptr m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		SceneInfoDescriptorSet_ptr m_sceneInfoDescriptorSet;
		ObjectInfoDescriptorSet_ptr m_objectInfoDescriptorSet;
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;
		TextureDescriptorSet_ptr m_textureDescriptorSet;
	};
}