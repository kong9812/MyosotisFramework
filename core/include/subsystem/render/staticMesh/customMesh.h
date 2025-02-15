// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"

namespace MyosotisFW::System::Render
{
	class CustomMesh : public StaticMesh
	{
	public:
		CustomMesh();
		~CustomMesh() {};

		const ObjectType GetObjectType() const override { return ObjectType::CustomMesh; }

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera) override;
		void BindCommandBuffer(VkCommandBuffer commandBuffer, RenderPipelineType pipelineType) override;
		//void BindDebugGUIElement() override {};

		void SetCustomMeshInfo(CustomMeshInfo customMeshInfo) { m_customMeshInfo = customMeshInfo; }

		glm::vec4 GetCullerData() override;
		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) override;
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};

		CustomMeshInfo m_customMeshInfo;
	};
	TYPEDEF_SHARED_PTR(CustomMesh)
		OBJECT_CAST_FUNCTION(CustomMesh)
}