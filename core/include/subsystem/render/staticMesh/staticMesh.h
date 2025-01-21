// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>

#include "classPointer.h"
#include "vkStruct.h"
#include "renderDevice.h"
#include "renderResources.h"
#include "camera.h"

namespace MyosotisFW::System::Render
{
	// todo.ファクトリーパターンにする予定
	// StaticMeshFactory
	// ・PrimitiveGeometryMesh
	// ・FBXMesh
	// ・...
	class StaticMesh
	{
	public:
		//  todo.初期化でrenderpipelineとdescriptorをとってくるのがいいかも
		StaticMesh(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		~StaticMesh();

		virtual void Update(const Camera::CameraBase& camera) {};
		virtual void BindCommandBuffer(VkCommandBuffer commandBuffer) {};

	protected:
		virtual void loadAssets() {};
		virtual void prepareUniformBuffers();
		virtual void prepareShaderStorageBuffers() {};

		// todo. descriptorsManagerに移す
		// todo. descriptorsはfactoryで作るのがいいかも
		virtual void prepareDescriptors();

		// todo. renderpipelineはfactoryで作るのがいいかも
		virtual void prepareRenderPipeline();

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// render pass
		VkRenderPass m_renderPass;

		// pipeline cache
		VkPipelineCache m_pipelineCache;

		// pipeline
		VkPipeline m_pipeline;

		// todo.この辺はfactoryで作るといいかも
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

		VkDescriptorSetLayout m_descriptorSetLayout; // パイプラインを作る時に必要！！

		// render pipeline
		VkPipelineLayout m_pipelineLayout;

		// vertex buffer
		Utility::Vulkan::Struct::Buffer m_vertexBuffer;
		// index buffer
		Utility::Vulkan::Struct::Buffer m_indexBuffer;

		// ubo
		// todo. UBOクラスを用意する(class StandardUBO)
		Utility::Vulkan::Struct::StaticMeshStandardUBO m_ubo;
		Utility::Vulkan::Struct::Buffer m_uboBuffer;
	};
	TYPEDEF_SHARED_PTR_ARGS(StaticMesh)
}
