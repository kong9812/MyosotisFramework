// Copyright (c) 2025 kong9812
#include "CustomMesh.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "VK_CreateInfo.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	CustomMesh::CustomMesh() : StaticMesh(),
		m_customMeshInfo({})
	{
		m_name = "CustomMesh";
	}

	void CustomMesh::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		__super::PrepareForRender(device, resources);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareShaderStorageBuffers();

		m_transform.scale = glm::vec3(1.0f);
		m_transform.rot = glm::vec3(-90.0f, 0.0f, 0.0f);

		// todo.検証処理
		m_isReady = true;
	}

	void CustomMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		__super::Update(updateData, camera);

		// todo.CustomMesh対応
		//m_staticMeshShaderObject.SSBO.standardSSBO.vertexMetaIndex = static_cast<uint32_t>(m_primitiveGeometryShape);

		if (!m_isReady) return;
	}

	rapidjson::Value CustomMesh::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value obj = __super::Serialize(allocator);

		obj.AddMember("meshPath", rapidjson::Value(m_customMeshInfo.m_meshPath.c_str(), allocator), allocator);

		return obj;
	}

	void CustomMesh::Deserialize(const rapidjson::Value& doc)
	{
		__super::Deserialize(doc);

		m_customMeshInfo.m_meshPath = doc["meshPath"].GetString();
	}

	void CustomMesh::loadAssets()
	{
		//std::vector<Mesh> meshes = m_resources->GetMeshVertex("Alicia\\Alicia_solid_MMD.FBX");
		std::vector<Mesh> meshes = m_resources->GetMeshVertex(m_customMeshInfo.m_meshPath);
		bool firstDataForAABB = true;

		for (int i = 0; i < LOD::Max; i++)
		{
			for (uint32_t meshIdx = 0; meshIdx < meshes.size(); meshIdx++)
			{
				m_vertexBuffer[i].resize(meshes.size());
				m_indexBuffer[i].resize(meshes.size());

				{// vertex
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(float) * meshes[meshIdx].vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer[i][meshIdx].buffer, &m_vertexBuffer[i][meshIdx].allocation, &m_vertexBuffer[i][meshIdx].allocationInfo));
					m_vertexBuffer[i][meshIdx].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer[i][meshIdx].buffer);
					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][meshIdx].allocation, &data));
					memcpy(data, meshes[meshIdx].vertex.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][meshIdx].allocation);
				}
				{// index
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * meshes[meshIdx].index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer[i][meshIdx].buffer, &m_indexBuffer[i][meshIdx].allocation, &m_indexBuffer[i][meshIdx].allocationInfo));
					m_indexBuffer[i][meshIdx].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer[i][meshIdx].buffer);

					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][meshIdx].allocation, &data));
					memcpy(data, meshes[meshIdx].index.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][meshIdx].allocation);

				}

				{// aabb
					if (firstDataForAABB)
					{
						m_aabbMin.x = meshes[meshIdx].min.x;
						m_aabbMin.y = meshes[meshIdx].min.y;
						m_aabbMin.z = meshes[meshIdx].min.z;
						m_aabbMax.x = meshes[meshIdx].max.x;
						m_aabbMax.y = meshes[meshIdx].max.y;
						m_aabbMax.z = meshes[meshIdx].max.z;
					}
					else
					{
						m_aabbMin.x = m_aabbMin.x < meshes[meshIdx].min.x ? m_aabbMin.x : meshes[meshIdx].min.x;
						m_aabbMin.y = m_aabbMin.y < meshes[meshIdx].min.y ? m_aabbMin.y : meshes[meshIdx].min.y;
						m_aabbMin.z = m_aabbMin.z < meshes[meshIdx].min.z ? m_aabbMin.z : meshes[meshIdx].min.z;
						m_aabbMax.x = m_aabbMax.x > meshes[meshIdx].max.x ? m_aabbMax.x : meshes[meshIdx].max.x;
						m_aabbMax.y = m_aabbMax.y > meshes[meshIdx].max.y ? m_aabbMax.y : meshes[meshIdx].max.y;
						m_aabbMax.z = m_aabbMax.z > meshes[meshIdx].max.z ? m_aabbMax.z : meshes[meshIdx].max.z;
					}
				}
			}
		}
	}
}