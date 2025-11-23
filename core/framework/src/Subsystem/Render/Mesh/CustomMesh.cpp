// Copyright (c) 2025 kong9812
#include "CustomMesh.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "VK_CreateInfo.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	CustomMesh::CustomMesh(const uint32_t objectID) : StaticMesh(objectID),
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

		// 一時対応
		std::vector<uint32_t> index{};
		for (const Mesh& mesh : meshes)
		{
			for (const Meshlet& meshlet : mesh.meshlet)
			{
				index.insert(index.end(), meshlet.primitives.begin(), meshlet.primitives.end());
			}
		}

		bool firstDataForAABB = true;

		for (int i = 0; i < LOD::Max; i++)
		{
			for (uint32_t meshIdx = 0; meshIdx < meshes.size(); meshIdx++)
			{
				std::vector<uint32_t> index{};
				const Mesh& mesh = meshes[meshIdx];
				for (const Meshlet& meshlet : mesh.meshlet)
				{
					index.insert(index.end(), meshlet.primitives.begin(), meshlet.primitives.end());
				}

				{// aabb
					if (firstDataForAABB)
					{
						m_aabbMin.x = meshes[meshIdx].meshInfo.AABBMin.x;
						m_aabbMin.y = meshes[meshIdx].meshInfo.AABBMin.y;
						m_aabbMin.z = meshes[meshIdx].meshInfo.AABBMin.z;
						m_aabbMax.x = meshes[meshIdx].meshInfo.AABBMax.x;
						m_aabbMax.y = meshes[meshIdx].meshInfo.AABBMax.y;
						m_aabbMax.z = meshes[meshIdx].meshInfo.AABBMax.z;
					}
					else
					{
						m_aabbMin.x = m_aabbMin.x < meshes[meshIdx].meshInfo.AABBMin.x ? m_aabbMin.x : meshes[meshIdx].meshInfo.AABBMin.x;
						m_aabbMin.y = m_aabbMin.y < meshes[meshIdx].meshInfo.AABBMin.y ? m_aabbMin.y : meshes[meshIdx].meshInfo.AABBMin.y;
						m_aabbMin.z = m_aabbMin.z < meshes[meshIdx].meshInfo.AABBMin.z ? m_aabbMin.z : meshes[meshIdx].meshInfo.AABBMin.z;
						m_aabbMax.x = m_aabbMax.x > meshes[meshIdx].meshInfo.AABBMax.x ? m_aabbMax.x : meshes[meshIdx].meshInfo.AABBMax.x;
						m_aabbMax.y = m_aabbMax.y > meshes[meshIdx].meshInfo.AABBMax.y ? m_aabbMax.y : meshes[meshIdx].meshInfo.AABBMax.y;
						m_aabbMax.z = m_aabbMax.z > meshes[meshIdx].meshInfo.AABBMax.z ? m_aabbMax.z : meshes[meshIdx].meshInfo.AABBMax.z;
					}
				}
			}
		}
	}
}