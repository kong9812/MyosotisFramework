// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class MeshShaderRenderPipeline : public RenderPipelineBase
	{
	public:
		MeshShaderRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors) {
		}
		~MeshShaderRenderPipeline();

		// テスト用後で消す
		struct {
			uint32_t testMeshletCount;
			uint32_t empty;
		}pushConstant;

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(MeshShaderRenderPipeline);
}