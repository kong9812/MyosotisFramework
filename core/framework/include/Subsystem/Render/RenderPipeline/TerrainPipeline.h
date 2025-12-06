// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"
#include "RawMeshData.h"

namespace MyosotisFW::System::Render
{
	class TerrainPipeline : public RenderPipelineBase
	{
	public:
		TerrainPipeline(const RenderDevice_ptr& device,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			RenderPipelineBase(device, sceneInfoDescriptorSet, objectInfoDescriptorSet, meshInfoDescriptorSet, textureDescriptorSet),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~TerrainPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(TerrainPipeline);
}