// Copyright (c) 2025 kong9812
#pragma once
#include "staticMesh.h"

namespace MyosotisFW::System::Render
{
	typedef struct
	{
		std::string m_meshPath;
	}CustomMeshInfo;

	class CustomMesh : public StaticMesh
	{
	public:
		CustomMesh();
		~CustomMesh() {};

		virtual const ObjectType GetObjectType() const override { return ObjectType::CustomMesh; }

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache) override;
		void Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera) override;
		void BindCommandBuffer(VkCommandBuffer commandBuffer) override;
		void BindDebugGUIElement() override {};

		void SetCustomMeshInfo(CustomMeshInfo customMeshInfo) { m_customMeshInfo = customMeshInfo; }

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) override;
	private:
		void loadAssets() override;
		void prepareUniformBuffers() override;
		void prepareShaderStorageBuffers() override {};

		void prepareDescriptors() override;
		void prepareRenderPipeline() override;

		CustomMeshInfo m_customMeshInfo;
	};
	TYPEDEF_SHARED_PTR(CustomMesh)
	OBJECT_CAST_FUNCTION(CustomMesh)
}