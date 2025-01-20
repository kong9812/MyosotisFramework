// Copyright (c) 2025 kong9812
#include "primitiveGeometry.h"

#include "vkCreateInfo.h"
#include "primitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache) :
		StaticMesh(device, resources, renderPass, pipelineCache)
	{
		// プリミティブジオメトリの作成
		loadAssets();
		prepareUniformBuffers();
		prepareShaderStorageBuffers();
		prepareDescriptors();
		prepareRenderPipeline();



	}

	PrimitiveGeometry::~PrimitiveGeometry()
	{

	}

	void PrimitiveGeometry::BindCommandBuffer(VkCommandBuffer commandBuffer)
	{
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, {0});
		//vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void PrimitiveGeometry::loadAssets()
	{
		Utility::Vulkan::Struct::Buffer stagingBuffer{};
		Utility::Vulkan::Struct::VertexAndIndex vertex = MyosotisFW::System::Render::Shape::createQuad(1.0f);
		uint32_t bufferSize = vertex.vertex.size() * static_cast<uint32_t>(sizeof(Utility::Vulkan::Struct::Vertex));
		m_device->CreateBuffer(
			stagingBuffer,
			bufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		memcpy(stagingBuffer.mapped, vertex.vertex.data(), bufferSize);
		vkUnmapMemory(*m_device, stagingBuffer.memory);
		vkDestroyBuffer(*m_device, stagingBuffer.buffer, m_device->GetAllocationCallbacks());
		vkFreeMemory(*m_device, stagingBuffer.memory, m_device->GetAllocationCallbacks());
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