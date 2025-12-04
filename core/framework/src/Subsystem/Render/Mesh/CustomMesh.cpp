// Copyright (c) 2025 kong9812
#include "CustomMesh.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "VK_CreateInfo.h"
#include "Camera.h"
#include "MeshInfoDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	CustomMesh::CustomMesh(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback) : StaticMesh(objectID, meshChangedCallback),
		m_customMeshInfo({})
	{
		m_name = "CustomMesh";
	}

	void CustomMesh::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet)
	{
		__super::PrepareForRender(device, resources, meshInfoDescriptorSet);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareShaderStorageBuffers();

		// todo.検証処理
		m_isReady = true;
	}

	void CustomMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		__super::Update(updateData, camera);
		if (!m_isReady) return;
	}

	rapidjson::Value CustomMesh::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value obj = __super::Serialize(allocator);

		obj.AddMember("meshName", rapidjson::Value(m_customMeshInfo.meshName.c_str(), allocator), allocator);

		return obj;
	}

	void CustomMesh::Deserialize(const rapidjson::Value& doc)
	{
		__super::Deserialize(doc);

		m_customMeshInfo.meshName = doc["meshName"].GetString();
	}

	void CustomMesh::loadAssets()
	{
		std::vector<Mesh> meshes = m_resources->GetMesh(m_customMeshInfo.meshName);
		std::vector<uint32_t> meshID = m_meshInfoDescriptorSet->AddCustomGeometry(m_customMeshInfo.meshName, meshes);
		m_meshCount = static_cast<uint32_t>(meshes.size());

		// VBDispatchInfoの作成
		for (uint32_t i = 0; i < m_meshCount; i++)
		{
			const MeshInfo meshInfo = m_meshInfoDescriptorSet->GetMeshInfo(meshID[i]);
			for (uint32_t j = 0; j < meshInfo.meshletCount; j++)
			{
				VBDispatchInfo vbDispatchInfo{};
				vbDispatchInfo.objectID = m_objectID;	// MObjectRegistryでセットされたobjectIDを使う
				vbDispatchInfo.meshID = meshID[i];		// meshIDそのままを使って、iではない！
				vbDispatchInfo.meshletID = j;			// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
				// vbDispatchInfo.bitFlags |= (1u << 0);	// 実験
				m_vbDispatchInfo.push_back(vbDispatchInfo);
			}
		}
	}
}