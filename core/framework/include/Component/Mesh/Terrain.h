// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "FilePath.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	class Terrain : public StaticMesh
	{
	public:
		Terrain(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback);
		~Terrain() {};

		struct MeshComponentInfo
		{
			FilePath terrainHeightmapName{};
		};
	public:

		const ComponentType GetType() const override { return ComponentType::Terrain; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;

		void SetMeshComponentInfo(const MeshComponentInfo MeshComponentInfo) { m_meshComponentInfo = MeshComponentInfo; loadAssets(); }
		const MeshComponentInfo& GetMeshComponentInfo() const { return m_meshComponentInfo; }

		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;

		MeshComponentInfo m_meshComponentInfo;

	public:
		static constexpr PropertyDesc::FilePathItem HeightMap = {
			AppInfo::g_assetRootFolder,		// basePass
			"*.png",						// filter
			AppInfo::g_terrainFolder		// defaultDir
		};

		// ComponentProperty
		static const PropertyTable& StaticPropertyTable()
		{
			static const PropertyDesc props[] = {
				MakeProperty(uuids::hashMaker(), "Heightmap", "Terrain",
					&HeightMap,
					PropertyDesc::PropertyFlags::None,
					+[](const void* obj)->PropertyDesc::PropertyValue
					{
						auto* o = static_cast<const Terrain*>(obj);
						return static_cast<FilePath>(o->m_meshComponentInfo.terrainHeightmapName);
					},
					+[](void* obj, const PropertyDesc::PropertyValue& v, PropertyDesc::ChangeReason cr)
					{
						auto* o = static_cast<Terrain*>(obj);
						o->m_meshComponentInfo.terrainHeightmapName = std::get<FilePath>(v);
						o->SetMeshComponentInfo(o->m_meshComponentInfo);
					}),
			};
			static const PropertyTable table{ &ComponentBase::StaticPropertyTable(), props, std::size(props) };
			return table;
		}
		const PropertyTable& GetPropertyTable() const override { return StaticPropertyTable(); }

	protected:
		virtual void OnPropertyChanged(uuids::uuid propertyID) {}
	};
	TYPEDEF_SHARED_PTR_ARGS(Terrain);
	OBJECT_CAST_FUNCTION(Terrain);
}