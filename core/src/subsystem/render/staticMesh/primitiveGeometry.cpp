// Copyright (c) 2025 kong9812
#include "PrimitiveGeometry.h"

#include "VK_CreateInfo.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry() : StaticMesh()
	{
		m_name = "プリミティブジオメトリ";
	}

	void PrimitiveGeometry::PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass)
	{
		__super::PrepareForRender(device, resources, renderPass);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareUniformBuffers();
		prepareShaderStorageBuffers();
		prepareDescriptors();
		prepareRenderPipeline();
		m_transfrom.scale = glm::vec3(5.0f);

		// todo.検証処理
		m_isReady = true;
	}

	void PrimitiveGeometry::Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera)
	{
		__super::Update(updateData, camera);

		if (camera)
		{
			m_staticMeshShaderObject.standardUBO.data.projection = camera->GetProjectionMatrix();
			m_staticMeshShaderObject.standardUBO.data.view = camera->GetViewMatrix();
			m_staticMeshShaderObject.standardUBO.data.cameraPos = glm::vec4(camera->GetCameraPos(), 0.0f);
		}
		m_staticMeshShaderObject.standardUBO.data.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transfrom.pos));
		m_staticMeshShaderObject.standardUBO.data.model = glm::scale(m_staticMeshShaderObject.standardUBO.data.model, glm::vec3(m_transfrom.scale));

		if (!m_isReady) return;
		memcpy(m_staticMeshShaderObject.standardUBO.buffer.allocationInfo.pMappedData, &m_staticMeshShaderObject.standardUBO.data, sizeof(m_staticMeshShaderObject.standardUBO.data));
	}

	void PrimitiveGeometry::BindCommandBuffer(VkCommandBuffer commandBuffer)
	{
		__super::BindCommandBuffer(commandBuffer);
	}

	glm::vec4 PrimitiveGeometry::GetCullerData()
	{
		return glm::vec4(m_transfrom.pos, 5.0f);
	}

	void PrimitiveGeometry::loadAssets()
	{
		Mesh vertex[LOD::Max] = {
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)),
		};

		for (int i = 0; i < LOD::Max; i++)
		{
			m_vertexBuffer[i].resize(1);
			m_indexBuffer[i].resize(1);

			{// vertex
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(float) * vertex[i].vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer[i][0].buffer, &m_vertexBuffer[i][0].allocation, &m_vertexBuffer[i][0].allocationInfo));
				m_vertexBuffer[i][0].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer[i][0].buffer);
				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][0].allocation, &data));
				memcpy(data, vertex[i].vertex.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i][0].allocation);
			}
			{// index
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * vertex[i].index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer[i][0].buffer, &m_indexBuffer[i][0].allocation, &m_indexBuffer[i][0].allocationInfo));
				m_indexBuffer[i][0].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer[i][0].buffer);

				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][0].allocation, &data));
				memcpy(data, vertex[i].index.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i][0].allocation);
			}
		}
	}

	void PrimitiveGeometry::prepareUniformBuffers()
	{
		__super::prepareUniformBuffers();
	}

	void PrimitiveGeometry::prepareDescriptors()
	{
		__super::prepareDescriptors();
	}

	void PrimitiveGeometry::prepareRenderPipeline()
	{
		__super::prepareRenderPipeline();
	}
}