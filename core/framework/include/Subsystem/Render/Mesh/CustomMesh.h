// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"

namespace MyosotisFW::System::Render
{
	class CustomMesh : public StaticMesh
	{
	public:
		CustomMesh(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback);
		~CustomMesh() {};

		const ComponentType GetType() const override { return ComponentType::CustomMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;

		void SetCustomMeshInfo(const CustomMeshInfo& customMeshInfo) { m_customMeshInfo = customMeshInfo; }
		const CustomMeshInfo& GetCustomMeshInfo() const { return m_customMeshInfo; }

		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};

		CustomMeshInfo m_customMeshInfo;
	};
	TYPEDEF_SHARED_PTR_ARGS(CustomMesh);
	OBJECT_CAST_FUNCTION(CustomMesh);
}