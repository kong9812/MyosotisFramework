// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class SkyboxPipeline : public RenderPipelineBase
	{
	public:
		SkyboxPipeline(const RenderDevice_ptr& device,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			RenderPipelineBase(device, sceneInfoDescriptorSet, objectInfoDescriptorSet, meshInfoDescriptorSet, textureDescriptorSet),
			pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~SkyboxPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct {
			uint32_t skyboxTextureID;
		}pushConstant;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(SkyboxPipeline);
}