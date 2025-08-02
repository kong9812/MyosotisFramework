// Copyright (c) 2025 kong9812
#include "InteriorObject.h"

#include "Camera.h"
#include "RenderDevice.h"
#include "RenderResources.h"

#include "VK_CreateInfo.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	InteriorObject::InteriorObject() : ComponentBase(),
		m_device(nullptr),
		m_resources(nullptr),
		m_vertexBuffer({}),
		m_indexBuffer({}),
		m_interiorObjectShaderObject({})
	{
		m_name = "InteriorObject";
	}

	InteriorObject::~InteriorObject()
	{
		if (m_interiorObjectShaderObject.cubeMap.sampler)
		{
			vkDestroySampler(*m_device, m_interiorObjectShaderObject.cubeMap.sampler, m_device->GetAllocationCallbacks());
		}

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer.buffer, m_vertexBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer.buffer, m_indexBuffer.allocation);
	}

	void InteriorObject::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		m_device = device;
		m_resources = resources;

		// プリミティブジオメトリの作成
		loadAssets();

		// todo.検証処理
		m_isReady = true;
	}

	void InteriorObject::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		m_interiorObjectShaderObject.SSBO.standardSSBO.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transform.pos));
		m_interiorObjectShaderObject.SSBO.standardSSBO.model = glm::rotate(m_interiorObjectShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_interiorObjectShaderObject.SSBO.standardSSBO.model = glm::rotate(m_interiorObjectShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_interiorObjectShaderObject.SSBO.standardSSBO.model = glm::rotate(m_interiorObjectShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_interiorObjectShaderObject.SSBO.standardSSBO.model = glm::scale(m_interiorObjectShaderObject.SSBO.standardSSBO.model, glm::vec3(m_transform.scale));
		m_interiorObjectShaderObject.SSBO.standardSSBO.obbData = GetWorldOBBData();
		m_interiorObjectShaderObject.SSBO.standardSSBO.renderID = m_renderID;

		if (!m_isReady) return;
	}

	void InteriorObject::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_interiorObjectShaderObject.shaderBase.pipelineLayout, 0, 1, &m_interiorObjectShaderObject.shaderBase.descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_interiorObjectShaderObject.shaderBase.pipeline);

		vkCmdPushConstants(commandBuffer, m_interiorObjectShaderObject.shaderBase.pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			0, static_cast<uint32_t>(sizeof(m_interiorObjectShaderObject.pushConstant)), &m_interiorObjectShaderObject.pushConstant);

		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer.allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
	}

	void InteriorObject::loadAssets()
	{
		Mesh vertex = MyosotisFW::System::Render::Shape::createShape(Shape::PrimitiveGeometryShape::Plane, 5.0f);
		bool firstDataForAABB = true;

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
		m_interiorObjectShaderObject.cubeMap = m_resources->GetCubeImage({
			"InteriorMappingTest\\right.png",
			"InteriorMappingTest\\left.png",
			"InteriorMappingTest\\top.png",
			"InteriorMappingTest\\bottom.png",
			"InteriorMappingTest\\front.png",
			"InteriorMappingTest\\back.png" });

		// sampler
		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_interiorObjectShaderObject.cubeMap.sampler));
	}
}