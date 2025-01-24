// Copyright (c) 2025 kong9812
#include "primitiveGeometry.h"

#include "vkCreateInfo.h"
#include "primitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry() : StaticMesh()
	{
		m_name = "プリミティブジオメトリ";
	}

	void PrimitiveGeometry::PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache)
	{
		__super::PrepareForRender(device, resources, renderPass, pipelineCache);

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

	void PrimitiveGeometry::Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera)
	{
		__super::Update(updateData, camera);

		if (camera)
		{
			m_ubo.projection = camera->GetProjectionMatrix();
			m_ubo.view = camera->GetViewMatrix();
			m_ubo.cameraPos = glm::vec4(camera->GetCameraPos(), 0.0f);
		}
		m_ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transfrom.pos));
		m_ubo.model = glm::scale(m_ubo.model, glm::vec3(m_transfrom.scale));

		if (!m_isReady) return;
		memcpy(m_uboBuffer.allocationInfo.pMappedData, &m_ubo, sizeof(m_ubo));
	}

	void PrimitiveGeometry::BindCommandBuffer(VkCommandBuffer commandBuffer)
	{
		if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer[m_currentLOD].buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer[m_currentLOD].buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer[m_currentLOD].allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
	}

	void PrimitiveGeometry::loadAssets()
	{
		Utility::Vulkan::Struct::Vertex vertex[LOD::Max] = {
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
			MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)),
		};

		m_vertexBuffer.resize(LOD::Max);
		m_indexBuffer.resize(LOD::Max);
		for (int i = 0; i < LOD::Max; i++)
		{
			{// vertex
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(float) * vertex[i].vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer[i].buffer, &m_vertexBuffer[i].allocation, &m_vertexBuffer[i].allocationInfo));
				m_vertexBuffer[i].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer[i].buffer);
				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i].allocation, &data));
				memcpy(data, vertex[i].vertex.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer[i].allocation);
			}
			{// index
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * vertex[i].index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer[i].buffer, &m_indexBuffer[i].allocation, &m_indexBuffer[i].allocationInfo));
				m_indexBuffer[i].descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer[i].buffer);

				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i].allocation, &data));
				memcpy(data, vertex[i].index.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer[i].allocation);
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