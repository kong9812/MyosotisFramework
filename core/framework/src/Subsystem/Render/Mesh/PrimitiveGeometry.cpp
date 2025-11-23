// Copyright (c) 2025 kong9812
#include "PrimitiveGeometry.h"

#include "VK_CreateInfo.h"

#include "Camera.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "MeshInfoDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry(const uint32_t objectID) : StaticMesh(objectID),
		m_primitiveGeometryShape(Shape::PrimitiveGeometryShape::Quad)
	{
		m_name = "PrimitiveGeometry";
	}

	void PrimitiveGeometry::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet)
	{
		__super::PrepareForRender(device, resources, meshInfoDescriptorSet);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareShaderStorageBuffers();

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
		Mesh mesh = MyosotisFW::System::Render::Shape::createShape(m_primitiveGeometryShape);
		uint32_t meshID = m_meshInfoDescriptorSet->AddPrimitiveGeometry(m_primitiveGeometryShape, mesh);

		uint32_t meshCount = 1;

		// VBDispatchInfoの作成
		for (uint32_t i = 0; i < meshCount; i++)
		{
			const MeshInfo meshInfo = m_meshInfoDescriptorSet->GetMeshInfo(meshID);
			for (uint32_t j = 0; j < meshInfo.meshletCount; j++)
			{
				VBDispatchInfo vbDispatchInfo;
				vbDispatchInfo.objectID = m_objectID;	// MObjectRegistryでセットされたobjectIDを使う
				vbDispatchInfo.meshID = meshID;			// meshIDそのままを使って、iではない！
				vbDispatchInfo.meshletID = j;			// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
				m_vbDispatchInfo.push_back(vbDispatchInfo);
			}
		}

		bool firstDataForAABB = true;

		// 一時対応
		std::vector<uint32_t> index{};
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			index.insert(index.end(), meshlet.primitives.begin(), meshlet.primitives.end());
		}

		{// aabb
			if (firstDataForAABB)
			{
				m_aabbMin.x = mesh.meshInfo.AABBMin.x;
				m_aabbMin.y = mesh.meshInfo.AABBMin.y;
				m_aabbMin.z = mesh.meshInfo.AABBMin.z;
				m_aabbMax.x = mesh.meshInfo.AABBMax.x;
				m_aabbMax.y = mesh.meshInfo.AABBMax.y;
				m_aabbMax.z = mesh.meshInfo.AABBMax.z;
			}
			else
			{
				m_aabbMin.x = m_aabbMin.x < mesh.meshInfo.AABBMin.x ? m_aabbMin.x : mesh.meshInfo.AABBMin.x;
				m_aabbMin.y = m_aabbMin.y < mesh.meshInfo.AABBMin.y ? m_aabbMin.y : mesh.meshInfo.AABBMin.y;
				m_aabbMin.z = m_aabbMin.z < mesh.meshInfo.AABBMin.z ? m_aabbMin.z : mesh.meshInfo.AABBMin.z;
				m_aabbMax.x = m_aabbMax.x > mesh.meshInfo.AABBMax.x ? m_aabbMax.x : mesh.meshInfo.AABBMax.x;
				m_aabbMax.y = m_aabbMax.y > mesh.meshInfo.AABBMax.y ? m_aabbMax.y : mesh.meshInfo.AABBMax.y;
				m_aabbMax.z = m_aabbMax.z > mesh.meshInfo.AABBMax.z ? m_aabbMax.z : mesh.meshInfo.AABBMax.z;
			}
		}
		//// 実験
		//m_staticMeshShaderObject.useNormalMap = true;
		//m_staticMeshShaderObject.normalMap = m_resources->GetImage("NormalMap.png");

		//// sampler
		//VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		//VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_staticMeshShaderObject.normalMap.sampler));
	}
}