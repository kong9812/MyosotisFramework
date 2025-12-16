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
		RenderPipelineBase(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			m_device(device),
			m_pipelineLayout(VK_NULL_HANDLE),
			m_pipeline(VK_NULL_HANDLE),
			m_renderDescriptors(renderDescriptors) {
			m_sceneInfoDescriptorSet = m_renderDescriptors->GetSceneInfoDescriptorSet();
			m_objectInfoDescriptorSet = m_renderDescriptors->GetObjectInfoDescriptorSet();
			m_meshInfoDescriptorSet = m_renderDescriptors->GetMeshInfoDescriptorSet();
			m_textureDescriptorSet = m_renderDescriptors->GetTextureDescriptorSet();
			m_rayTracingDescriptorSet = m_renderDescriptors->GetRayTracingDescriptorSet();
		}
		virtual ~RenderPipelineBase() = default;

		virtual void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) {};

	protected:
		virtual void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) {};

		RenderDevice_ptr m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		RenderDescriptors_ptr m_renderDescriptors;

		SceneInfoDescriptorSet_ptr m_sceneInfoDescriptorSet;
		ObjectInfoDescriptorSet_ptr m_objectInfoDescriptorSet;
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;
		TextureDescriptorSet_ptr m_textureDescriptorSet;
		RayTracingDescriptorSet_ptr m_rayTracingDescriptorSet;
	};
}