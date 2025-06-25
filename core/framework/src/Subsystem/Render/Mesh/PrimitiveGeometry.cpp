// Copyright (c) 2025 kong9812
#include "PrimitiveGeometry.h"

#include "VK_CreateInfo.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry() : StaticMesh()
	{
		m_name = "プリミティブジオメトリ";
	}

	void PrimitiveGeometry::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		__super::PrepareForRender(device, resources);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareShaderStorageBuffers();

		// todo.検証処理
		m_isReady = true;
	}

	void PrimitiveGeometry::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		__super::Update(updateData, camera);

		if (camera)
		{
			m_staticMeshShaderObject.standardUBO.data.projection = camera->GetProjectionMatrix();
			m_staticMeshShaderObject.standardUBO.data.view = camera->GetViewMatrix();
		}
		m_staticMeshShaderObject.standardUBO.data.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transfrom.pos));
		m_staticMeshShaderObject.standardUBO.data.model = glm::rotate(m_staticMeshShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_staticMeshShaderObject.standardUBO.data.model = glm::rotate(m_staticMeshShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_staticMeshShaderObject.standardUBO.data.model = glm::rotate(m_staticMeshShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_staticMeshShaderObject.standardUBO.data.model = glm::scale(m_staticMeshShaderObject.standardUBO.data.model, glm::vec3(m_transfrom.scale));

		if (!m_isReady) return;
		memcpy(m_staticMeshShaderObject.standardUBO.buffer.allocationInfo.pMappedData, &m_staticMeshShaderObject.standardUBO.data, sizeof(m_staticMeshShaderObject.standardUBO.data));
	}

	void PrimitiveGeometry::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType)
	{
		__super::BindCommandBuffer(commandBuffer, pipelineType);
	}

	glm::vec4 PrimitiveGeometry::GetCullerData()
	{
		return glm::vec4(m_transfrom.pos, 2.5f);
	}

	rapidjson::Value PrimitiveGeometry::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json = __super::Serialize(allocator);
		json.AddMember("primitiveGeometryShape", static_cast<uint32_t>(m_primitiveGeometryShape), allocator);
		return json;
	}

	void PrimitiveGeometry::Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject)
	{
		__super::Deserialize(doc, createObject);
		m_primitiveGeometryShape = static_cast<Shape::PrimitiveGeometryShape>(doc["primitiveGeometryShape"].GetUint());
	}

	void PrimitiveGeometry::loadAssets()
	{
		Mesh vertex = MyosotisFW::System::Render::Shape::createShape(m_primitiveGeometryShape, 5.0f, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		bool firstDataForAABB = true;

		for (int i = 0; i < LOD::Max; i++)
		{
			m_vertexBuffer[i].resize(1);
			m_indexBuffer[i].resize(1);

			{// vertex
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(float) * vertex.vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer[i][0].buffer, &m_vertexBuffer[i][0].allocation, &m_vertexBuffer[i][0].allocationInfo));
				m_vertexBuffer[i][0].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer[i][0].buffer);
				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][0].allocation, &data));
				memcpy(data, vertex.vertex.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][0].allocation);
			}
			{// index
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * vertex.index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer[i][0].buffer, &m_indexBuffer[i][0].allocation, &m_indexBuffer[i][0].allocationInfo));
				m_indexBuffer[i][0].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer[i][0].buffer);

				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][0].allocation, &data));
				memcpy(data, vertex.index.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][0].allocation);
			}
		}

		{// aabb
			if (firstDataForAABB)
			{
				m_aabbMin.x = vertex.min.x;
				m_aabbMin.y = vertex.min.y;
				m_aabbMin.z = vertex.min.z;
				m_aabbMax.x = vertex.max.x;
				m_aabbMax.y = vertex.max.y;
				m_aabbMax.z = vertex.max.z;
			}
			else
			{
				m_aabbMin.x = m_aabbMin.x < vertex.min.x ? m_aabbMin.x : vertex.min.x;
				m_aabbMin.y = m_aabbMin.y < vertex.min.y ? m_aabbMin.y : vertex.min.y;
				m_aabbMin.z = m_aabbMin.z < vertex.min.z ? m_aabbMin.z : vertex.min.z;
				m_aabbMax.x = m_aabbMax.x > vertex.max.x ? m_aabbMax.x : vertex.max.x;
				m_aabbMax.y = m_aabbMax.y > vertex.max.y ? m_aabbMax.y : vertex.max.y;
				m_aabbMax.z = m_aabbMax.z > vertex.max.z ? m_aabbMax.z : vertex.max.z;
			}
		}
		// 実験
		m_staticMeshShaderObject.standardUBO.useNormalMap = true;
		m_staticMeshShaderObject.standardUBO.normalMap = m_resources->GetImage("NormalMap.png");
		// sampler
		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_staticMeshShaderObject.standardUBO.normalMap.sampler));
	}
}