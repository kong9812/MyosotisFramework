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
			pushConstant({}),
			m_hiZSamplerID(0),
			m_primaryDepthSamplerID(0),
			m_meshletSize(0),
			m_vkCmdDrawMeshTasksEXT(VK_NULL_HANDLE) {
		}
		~TerrainPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct {
			float hiZMipLevelMax;
			uint32_t hiZSamplerID;
			uint32_t vbDispatchInfoCount;
			//uint32_t checkFalseNegativeMesh;
		}pushConstant;

		uint32_t m_meshletSize;

		uint32_t m_hiZSamplerID;
		uint32_t m_primaryDepthSamplerID;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(TerrainPipeline);
}