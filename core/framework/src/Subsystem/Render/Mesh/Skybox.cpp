// Copyright (c) 2025 kong9812
#include "Skybox.h"

#include "VK_CreateInfo.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	Skybox::Skybox() : ObjectBase()
	{
		m_name = "スカイボックス";
		m_skyboxShaderObject.standardUBO.cubemap.sampler = VK_NULL_HANDLE;
	}

	Skybox::~Skybox()
	{
		if (m_skyboxShaderObject.standardUBO.cubemap.sampler)
		{
			vkDestroySampler(*m_device, m_skyboxShaderObject.standardUBO.cubemap.sampler, m_device->GetAllocationCallbacks());
		}

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer.buffer, m_vertexBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer.buffer, m_indexBuffer.allocation);

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_skyboxShaderObject.standardUBO.buffer.buffer, m_skyboxShaderObject.standardUBO.buffer.allocation);
	}

	void Skybox::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		m_device = device;
		m_resources = resources;

		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			m_skyboxShaderObject.standardUBO.data,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_skyboxShaderObject.standardUBO.buffer.buffer,
			m_skyboxShaderObject.standardUBO.buffer.allocation,
			m_skyboxShaderObject.standardUBO.buffer.allocationInfo,
			m_skyboxShaderObject.standardUBO.buffer.descriptor);

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void Skybox::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		if (camera)
		{
			m_skyboxShaderObject.standardUBO.data.projection = camera->GetProjectionMatrix();
			m_skyboxShaderObject.standardUBO.data.view = camera->GetViewMatrix();
		}
		m_skyboxShaderObject.standardUBO.data.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transfrom.pos));
		m_skyboxShaderObject.standardUBO.data.model = glm::rotate(m_skyboxShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_skyboxShaderObject.standardUBO.data.model = glm::rotate(m_skyboxShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_skyboxShaderObject.standardUBO.data.model = glm::rotate(m_skyboxShaderObject.standardUBO.data.model, glm::radians(m_transfrom.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_skyboxShaderObject.standardUBO.data.model = glm::scale(m_skyboxShaderObject.standardUBO.data.model, glm::vec3(m_transfrom.scale));

		if (!m_isReady) return;
		memcpy(m_skyboxShaderObject.standardUBO.buffer.allocationInfo.pMappedData, &m_skyboxShaderObject.standardUBO.data, sizeof(m_skyboxShaderObject.standardUBO.data));
	}

	void Skybox::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxShaderObject.shaderBase.pipelineLayout, 0, 1, &m_skyboxShaderObject.shaderBase.descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxShaderObject.shaderBase.pipeline);

		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer.allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
	}

	void Skybox::loadAssets()
	{
		Mesh vertex = MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

		{// vertex
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(float) * vertex.vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer.buffer, &m_vertexBuffer.allocation, &m_vertexBuffer.allocationInfo));
			m_vertexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer.buffer);
			// mapping
			void* data{};
			VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer.allocation, &data));
			memcpy(data, vertex.vertex.data(), bufferCreateInfo.size);
			vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer.allocation);
		}
		{// index
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * vertex.index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer.buffer, &m_indexBuffer.allocation, &m_indexBuffer.allocationInfo));
			m_indexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer.buffer);

			// mapping
			void* data{};
			VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation, &data));
			memcpy(data, vertex.index.data(), bufferCreateInfo.size);
			vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation);
		}


		// 実験
		m_skyboxShaderObject.standardUBO.cubemap = m_resources->GetCubeImage({
			"sky\\right.png",
			"sky\\left.png",
			"sky\\top.png",
			"sky\\bottom.png",
			"sky\\front.png",
			"sky\\back.png" });

		// sampler
		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_skyboxShaderObject.standardUBO.cubemap.sampler));
	}
}