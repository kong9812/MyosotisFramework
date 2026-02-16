// Copyright (c) 2025 kong9812
#include "PrimitiveGeometry.h"

#include "VK_CreateInfo.h"

#include "Camera.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "MeshInfoDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback) : StaticMesh(objectID, meshChangedCallback),
		m_meshComponentInfo()
	{
		m_name = "PrimitiveGeometry";
	}

	void PrimitiveGeometry::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		StaticMesh::PrepareForRender(device, resources);

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void PrimitiveGeometry::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		StaticMesh::Update(updateData, camera);
		//m_staticMeshShaderObject.SSBO.standardSSBO.meshDataIndex = static_cast<uint32_t>(m_meshComponentInfo.primitiveGeometryShape);

		if (!m_isReady) return;
	}

	rapidjson::Value PrimitiveGeometry::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json = StaticMesh::Serialize(allocator);
		json.AddMember("primitiveGeometryShape", static_cast<uint32_t>(m_meshComponentInfo.primitiveGeometryShape), allocator);
		return json;
	}

	void PrimitiveGeometry::Deserialize(const rapidjson::Value& doc)
	{
		StaticMesh::Deserialize(doc);
		m_meshComponentInfo.primitiveGeometryShape = static_cast<Shape::PrimitiveGeometryShape>(doc["primitiveGeometryShape"].GetUint());
		loadAssets();
	}

	void PrimitiveGeometry::loadAssets()
	{
		StaticMesh::loadAssets();

		m_meshCount = 0;
		m_meshID.clear();
		m_vbDispatchInfo.clear();
		m_aabbMin = glm::vec3(FLT_MAX);
		m_aabbMax = glm::vec3(-FLT_MAX);

		if (m_meshComponentInfo.primitiveGeometryShape == Shape::PrimitiveGeometryShape::UNDEFINED) return;

		// ここで何とかしてVBDispatchInfoを作らないといけない!!
		MeshHandle meshHandle = m_resources->GetPrimitiveGeometryMesh(m_meshComponentInfo.primitiveGeometryShape);
		std::shared_ptr<const Mesh> mesh = meshHandle.lock();
		const MeshInfo meshInfo = mesh->meshInfo;
		m_meshCount = 1;
		m_meshID.push_back(meshInfo.meshID);
		// VBDispatchInfoの作成
		for (uint32_t j = 0; j < meshInfo.meshletCount; j++)
		{
			VBDispatchInfo vbDispatchInfo{};
			vbDispatchInfo.objectID = m_objectID;			// MObjectRegistryでセットされたobjectIDを使う
			vbDispatchInfo.meshID = meshInfo.meshID;	// meshIDそのままを使って、iではない！
			vbDispatchInfo.meshletID = j;					// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
			m_vbDispatchInfo.push_back(vbDispatchInfo);
		}

		// aabb
		m_aabbMin = glm::min(m_aabbMin, glm::vec3(meshInfo.AABBMin));
		m_aabbMax = glm::max(m_aabbMax, glm::vec3(meshInfo.AABBMax));
	}
}