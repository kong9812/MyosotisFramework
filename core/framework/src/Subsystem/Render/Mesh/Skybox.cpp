// Copyright (c) 2025 kong9812
#include "Skybox.h"

#include "RenderDevice.h"
#include "RenderResources.h"
#include "Camera.h"

#include "VK_CreateInfo.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	Skybox::Skybox() : ComponentBase(),
		m_device(nullptr),
		m_resources(nullptr),
		m_vertexBuffer({}),
		m_indexBuffer({}),
		m_skyboxShaderObject({})
	{
		m_name = "Skybox";
		m_skyboxShaderObject.cubeMap.sampler = VK_NULL_HANDLE;
	}

	Skybox::~Skybox()
	{
		if (m_skyboxShaderObject.cubeMap.sampler)
		{
			vkDestroySampler(*m_device, m_skyboxShaderObject.cubeMap.sampler, m_device->GetAllocationCallbacks());
		}

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer.buffer, m_vertexBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer.buffer, m_indexBuffer.allocation);
	}

	void Skybox::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		m_device = device;
		m_resources = resources;

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void Skybox::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		m_skyboxShaderObject.SSBO.standardSSBO.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transform.pos));
		m_skyboxShaderObject.SSBO.standardSSBO.model = glm::rotate(m_skyboxShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_skyboxShaderObject.SSBO.standardSSBO.model = glm::rotate(m_skyboxShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_skyboxShaderObject.SSBO.standardSSBO.model = glm::rotate(m_skyboxShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_skyboxShaderObject.SSBO.standardSSBO.model = glm::scale(m_skyboxShaderObject.SSBO.standardSSBO.model, glm::vec3(m_transform.scale));
		m_skyboxShaderObject.SSBO.standardSSBO.obbData = GetWorldOBBData();
		m_skyboxShaderObject.SSBO.standardSSBO.renderID = m_renderID;

		if (!m_isReady) return;
	}

	void Skybox::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxShaderObject.shaderBase.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxShaderObject.shaderBase.pipelineLayout, 0, 1, &m_skyboxShaderObject.shaderBase.descriptorSet, 0, nullptr);
		vkCmdPushConstants(commandBuffer, m_skyboxShaderObject.shaderBase.pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			static_cast<uint32_t>(sizeof(m_skyboxShaderObject.pushConstant)), &m_skyboxShaderObject.pushConstant);
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer.allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
	}

	void Skybox::loadAssets()
	{
		Mesh vertex = MyosotisFW::System::Render::Shape::createQuad(1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

		// 一時対応
		std::vector<uint32_t> index{};
		for (const Meshlet& meshlet : vertex.meshlet)
		{
			index.insert(index.end(), meshlet.primitives.begin(), meshlet.primitives.end());
		}

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
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer.buffer, &m_indexBuffer.allocation, &m_indexBuffer.allocationInfo));
			m_indexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer.buffer);

			// mapping
			void* data{};
			VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation, &data));
			memcpy(data, index.data(), bufferCreateInfo.size);
			vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation);
		}

		// 実験
		m_skyboxShaderObject.cubeMap = m_resources->GetCubeImage({
			"sky\\right.png",
			"sky\\left.png",
			"sky\\top.png",
			"sky\\bottom.png",
			"sky\\front.png",
			"sky\\back.png" });

		// sampler
		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_skyboxShaderObject.cubeMap.sampler));
	}
}