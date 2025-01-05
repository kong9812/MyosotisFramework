// Copyright (c) 2025 kong9812
// Camera interface class
#pragma once
#include <vulkan/vulkan.h>

#include "vkStruct.h"
#include "renderDevice.h"

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
		StaticMesh(RenderDevice_prt device);
		~StaticMesh();

	protected:
		// todo. descriptorsManagerに移す
		// todo. descriptorsはfactoryで作るのがいいかも
		void prepareDescriptors();

		// todo. renderpipelineはfactoryで作るのがいいかも
		void prepareRenderPipeline();

		// render device
		RenderDevice_prt m_device;

		// todo. UBOクラスを用意する(class StandardUBO)
		Utility::Vulkan::Struct::StaticMeshStandardUBO m_ubo;

		// todo.この辺はfactoryで作るといいかも
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

		VkDescriptorSetLayout m_descriptorSetLayout; // パイプラインを作る時に必要！！

		// render pipeline
		VkPipelineLayout m_pipelineLayout;

		// ubo
		VkDescriptorBufferInfo m_uboDescriptor;
	};
}
