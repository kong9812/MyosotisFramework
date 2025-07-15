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

		const ComponentType GetType() const override { return ComponentType::CustomMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType) override;

		void SetCustomMeshInfo(const CustomMeshInfo& customMeshInfo) { m_customMeshInfo = customMeshInfo; }

		glm::vec4 GetCullerData() override;
		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};

		CustomMeshInfo m_customMeshInfo;
	};
	TYPEDEF_SHARED_PTR(CustomMesh)
		OBJECT_CAST_FUNCTION(CustomMesh)
}