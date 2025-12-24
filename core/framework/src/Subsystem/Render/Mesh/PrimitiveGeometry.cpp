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
		m_primitiveGeometryShape(Shape::PrimitiveGeometryShape::Quad)
	{
		m_name = "PrimitiveGeometry";
	}

	void PrimitiveGeometry::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet)
	{
		__super::PrepareForRender(device, resources, meshInfoDescriptorSet);

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void PrimitiveGeometry::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		__super::Update(updateData, camera);
		//m_staticMeshShaderObject.SSBO.standardSSBO.meshDataIndex = static_cast<uint32_t>(m_primitiveGeometryShape);

		if (!m_isReady) return;
	}

	rapidjson::Value PrimitiveGeometry::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json = __super::Serialize(allocator);
		json.AddMember("primitiveGeometryShape", static_cast<uint32_t>(m_primitiveGeometryShape), allocator);
		return json;
	}

	void PrimitiveGeometry::Deserialize(const rapidjson::Value& doc)
	{
		__super::Deserialize(doc);
		m_primitiveGeometryShape = static_cast<Shape::PrimitiveGeometryShape>(doc["primitiveGeometryShape"].GetUint());
	}

	void PrimitiveGeometry::loadAssets()
	{
		// ここで何とかしてVBDispatchInfoを作らないといけない!!
		MeshHandle meshHandle = m_resources->GetPrimitiveGeometryMesh(m_primitiveGeometryShape);
		std::shared_ptr<const Mesh> mesh = meshHandle.lock();
		m_meshCount = 1;
		m_meshID.push_back(mesh->meshInfo.meshID);
		// VBDispatchInfoの作成
		for (uint32_t j = 0; j < mesh->meshInfo.meshletCount; j++)
		{
			VBDispatchInfo vbDispatchInfo{};
			vbDispatchInfo.objectID = m_objectID;			// MObjectRegistryでセットされたobjectIDを使う
			vbDispatchInfo.meshID = mesh->meshInfo.meshID;	// meshIDそのままを使って、iではない！
			vbDispatchInfo.meshletID = j;					// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
			m_vbDispatchInfo.push_back(vbDispatchInfo);
		}

		m_tlasInstance->active = true;
	}
}