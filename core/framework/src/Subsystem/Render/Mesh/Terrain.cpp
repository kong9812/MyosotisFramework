// Copyright (c) 2025 kong9812
#include "Terrain.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "VK_CreateInfo.h"
#include "Camera.h"
#include "MeshInfoDescriptorSet.h"
#include "TerrainIo.h"

namespace MyosotisFW::System::Render
{
	Terrain::Terrain(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback) : StaticMesh(objectID, meshChangedCallback),
		m_meshComponentInfo({})
	{
		m_name = "Terrain";
	}

	void Terrain::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet)
	{
		__super::PrepareForRender(device, resources, meshInfoDescriptorSet);

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void Terrain::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		__super::Update(updateData, camera);
		if (!m_isReady) return;
	}

	rapidjson::Value Terrain::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value obj = __super::Serialize(allocator);

		obj.AddMember("meshName", rapidjson::Value(m_meshComponentInfo.terrainHeightmapName.c_str(), allocator), allocator);

		return obj;
	}

	void Terrain::Deserialize(const rapidjson::Value& doc)
	{
		__super::Deserialize(doc);

		m_meshComponentInfo.terrainHeightmapName = doc["meshName"].GetString();
	}

	void Terrain::loadAssets()
	{
		MeshesHandle meshesHandle = m_resources->GetTerrainMesh(m_meshComponentInfo.terrainHeightmapName);
		m_meshCount = static_cast<uint32_t>(meshesHandle.size());
		// VBDispatchInfoの作成
		for (uint32_t i = 0; i < m_meshCount; i++)
		{
			std::shared_ptr<const Mesh> mesh = meshesHandle[i].lock();
			const MeshInfo meshInfo = mesh->meshInfo;
			m_meshID.push_back(meshInfo.meshID);
			for (uint32_t j = 0; j < meshInfo.meshletCount; j++)
			{
				VBDispatchInfo vbDispatchInfo{};
				vbDispatchInfo.objectID = m_objectID;		// MObjectRegistryでセットされたobjectIDを使う
				vbDispatchInfo.meshID = meshInfo.meshID;	// meshIDそのままを使って、iではない！
				vbDispatchInfo.meshletID = j;				// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
				// vbDispatchInfo.bitFlags |= (1u << 0);	// 実験
				m_vbDispatchInfo.push_back(vbDispatchInfo);
			}
		}

		m_tlasInstanceInfo->active = true;
	}
}