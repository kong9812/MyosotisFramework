// Copyright (c) 2025 kong9812
#include "customMesh.h"
#include "renderResources.h"
#include "vkCreateInfo.h"

namespace MyosotisFW::System::Render
{
	CustomMesh::CustomMesh() : StaticMesh()
	{
		m_name = "カスタムメッシュ";
	}

	void CustomMesh::PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache)
	{
		__super::PrepareForRender(device, resources, renderPass, pipelineCache);

		// プリミティブジオメトリの作成
		loadAssets();
		prepareUniformBuffers();
		prepareShaderStorageBuffers();
		prepareDescriptors();
		prepareRenderPipeline();

		m_transfrom.scale = glm::vec3(0.05f);

		// todo.検証処理
		m_isReady = true;
	}

	void CustomMesh::Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera)
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

	void CustomMesh::BindCommandBuffer(VkCommandBuffer commandBuffer)
	{
		if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		const VkDeviceSize offsets[1] = { 0 };
		for (uint32_t meshIdx = 0; meshIdx < m_vertexBuffer[m_currentLOD].size(); meshIdx++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer[m_currentLOD][meshIdx].buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
		}
	}

	void CustomMesh::loadAssets()
	{
		//std::vector<Utility::Vulkan::Struct::Mesh> meshes = m_resources->GetMeshVertex("Alicia\\Alicia_solid_MMD.FBX");
		std::vector<Utility::Vulkan::Struct::Mesh> meshes = m_resources->GetMeshVertex("Test\\a.FBX");

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
			}
		}
	}

	void CustomMesh::prepareUniformBuffers()
	{
		__super::prepareUniformBuffers();
	}

	void CustomMesh::prepareDescriptors()
	{
		__super::prepareDescriptors();
	}

	void CustomMesh::prepareRenderPipeline()
	{
		__super::prepareRenderPipeline();
	}
}