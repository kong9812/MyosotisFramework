// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"

namespace MyosotisFW::System::Render
{
	class Terrain : public StaticMesh
	{
	public:
		Terrain(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback);
		~Terrain() {};

		struct MeshComponentInfo
		{
			std::string terrainHeightmapName;
		};
	public:

		const ComponentType GetType() const override { return ComponentType::Terrain; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;

		void SetMeshComponentInfo(const MeshComponentInfo& MeshComponentInfo) { m_meshComponentInfo = MeshComponentInfo; }
		const MeshComponentInfo& GetMeshComponentInfo() const { return m_meshComponentInfo; }

		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;

		MeshComponentInfo m_meshComponentInfo;
	};
	TYPEDEF_SHARED_PTR_ARGS(Terrain);
	OBJECT_CAST_FUNCTION(Terrain);
}