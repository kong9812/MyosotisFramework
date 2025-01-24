// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include <vulkan/vulkan.h>

#include "objectCast.h"
#include "objectBase.h"
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
	class StaticMesh : public ObjectBase
	{
	public:
		//  todo.初期化でrenderpipelineとdescriptorをとってくるのがいいかも
		StaticMesh();
		~StaticMesh();

		typedef enum {
			Hide = -1,
			VeryClose,
			Close,
			Far,
			Max
		} LOD;

		virtual void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		virtual void Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera);
		virtual void BindCommandBuffer(VkCommandBuffer commandBuffer) override {};
		virtual void BindDebugGUIElement() override {};

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) override { __super::Deserialize(doc, createObject); }
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
		std::vector<Utility::Vulkan::Struct::Buffer> m_vertexBuffer;
		// index buffer
		std::vector<Utility::Vulkan::Struct::Buffer> m_indexBuffer;

		// lod
		LOD m_currentLOD;
		std::array<float, LOD::Max> m_lodDistances;

		// ubo
		// todo. UBOクラスを用意する(class StandardUBO)
		Utility::Vulkan::Struct::StaticMeshStandardUBO m_ubo;
		Utility::Vulkan::Struct::Buffer m_uboBuffer;
	};
	TYPEDEF_SHARED_PTR(StaticMesh)
	OBJECT_CAST_FUNCTION(StaticMesh)
}
