// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class VisibilityBufferRenderPhase1Pipeline : public RenderPipelineBase
	{
	public:
		VisibilityBufferRenderPhase1Pipeline(const RenderDevice_ptr& device,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			RenderPipelineBase(device, sceneInfoDescriptorSet, objectInfoDescriptorSet, meshInfoDescriptorSet, textureDescriptorSet) {
		}
		~VisibilityBufferRenderPhase1Pipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t meshletCount);

	private:
		struct {
			uint32_t hiZSamplerID;
			uint32_t checkFalseNegativeMesh;
		}pushConstant;

		uint32_t m_hiZSamplerID;
		uint32_t m_primaryDepthSamplerID;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(VisibilityBufferRenderPhase1Pipeline);
}