// Copyright (c) 2025 kong9812
#include "LightmapBakingPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "MObject.h"
#include "TerrainIo.h"
#include "ImageIo.h"

#include "PrimitiveGeometryShape.h"

#include "CustomMesh.h"
#include "PrimitiveGeometry.h"
#include "Terrain.h"

namespace MyosotisFW::System::Render
{
	LightmapBakingPipeline::~LightmapBakingPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());

		for (Buffer& buffer : m_vertexBuffer)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		}
		for (Buffer& buffer : m_indexBuffer)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		}
	}

	void LightmapBakingPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);
		pushConstant.atlasSize = glm::ivec2(AppInfo::g_lightmapSize, AppInfo::g_lightmapSize);
	}

	void LightmapBakingPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		if ((m_vertexBuffer[m_vertexBuffer.size() - 1].buffer == VK_NULL_HANDLE) || (m_indexBuffer[m_indexBuffer.size() - 1].buffer == VK_NULL_HANDLE)) return;

		Buffer& currentVertexBuffer = m_vertexBuffer[m_vertexBuffer.size() - 1];
		Buffer& currentIndexBuffer = m_indexBuffer[m_indexBuffer.size() - 1];

		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
			0,
			static_cast<uint32_t>(sizeof(pushConstant)), &pushConstant);
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &currentVertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, currentIndexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, currentIndexBuffer.allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
	}

	bool LightmapBakingPipeline::NextObject(const RenderResources_ptr& resources, const MObject_ptr& object)
	{
		{// constom mesh
			ComponentBase_ptr ptr = object->FindComponent(ComponentType::CustomMesh);
			if (ptr)
			{
				CustomMesh_ptr customMesh = Object_CastToCustomMesh(ptr);

				// Buffer
				Buffer* vertexBuffer = &m_vertexBuffer.emplace_back(Buffer());
				Buffer* indexBuffer = &m_indexBuffer.emplace_back(Buffer());

				// vertex buffer
				std::vector<Mesh> meshes = resources->GetMesh(customMesh->GetMeshComponentInfo().meshName);
				{// vertex
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(VertexData) * meshes[0].vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &vertexBuffer->buffer, &vertexBuffer->allocation, &vertexBuffer->allocationInfo));
					vertexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(vertexBuffer->buffer);
					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation, &data));
					memcpy(data, meshes[0].vertex.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation);
				}
				{// index
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * meshes[0].index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &indexBuffer->buffer, &indexBuffer->allocation, &indexBuffer->allocationInfo));
					indexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(indexBuffer->buffer);

					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation, &data));
					memcpy(data, meshes[0].index.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation);
				}
				pushConstant.size = meshes[0].meshInfo.atlasSize;
				ASSERT(allocateLightmapAtlas(pushConstant.size, pushConstant.offset), "Failed to alloc from lightmap.");
				return true;
			}
		}

		{// primitive geometry
			ComponentBase_ptr ptr = object->FindComponent(ComponentType::PrimitiveGeometryMesh);
			if (ptr)
			{
				PrimitiveGeometry_ptr primitiveGeom = Object_CastToPrimitiveGeometry(ptr);

				// Buffer
				Buffer* vertexBuffer = &m_vertexBuffer.emplace_back(Buffer());
				Buffer* indexBuffer = &m_indexBuffer.emplace_back(Buffer());

				// vertex buffer
				Mesh mesh = Shape::createShape(primitiveGeom->GetPrimitiveGeometryShape());
				{// vertex
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(VertexData) * mesh.vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &vertexBuffer->buffer, &vertexBuffer->allocation, &vertexBuffer->allocationInfo));
					vertexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(vertexBuffer->buffer);
					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation, &data));
					memcpy(data, mesh.vertex.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation);
				}
				{// index
					VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * mesh.index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
					VmaAllocationCreateInfo allocationCreateInfo{};
					allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
					VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &indexBuffer->buffer, &indexBuffer->allocation, &indexBuffer->allocationInfo));
					indexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(indexBuffer->buffer);

					// mapping
					void* data{};
					VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation, &data));
					memcpy(data, mesh.index.data(), bufferCreateInfo.size);
					vmaUnmapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation);
				}
				pushConstant.size = mesh.meshInfo.atlasSize;
				ASSERT(allocateLightmapAtlas(pushConstant.size, pushConstant.offset), "Failed to alloc from lightmap.");
				return true;
			}
		}

		//{// Terrain
		//	ComponentBase_ptr ptr = object->FindComponent(ComponentType::Terrain);
		//	if (ptr)
		//	{
		//		Terrain_ptr terrain = Object_CastToTerrain(ptr);

		//		// Buffer
		//		Buffer* vertexBuffer = &m_vertexBuffer.emplace_back(Buffer());
		//		Buffer* indexBuffer = &m_indexBuffer.emplace_back(Buffer());

		//		// vertex buffer
		//		std::vector<Mesh> meshes = Utility::Loader::loadTerrainMesh(terrain->GetMeshComponentInfo().terrainHeightmapName);
		//		{// vertex
		//			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(VertexData) * meshes[0].vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		//			VmaAllocationCreateInfo allocationCreateInfo{};
		//			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
		//			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &vertexBuffer->buffer, &vertexBuffer->allocation, &vertexBuffer->allocationInfo));
		//			vertexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(vertexBuffer->buffer);
		//			// mapping
		//			void* data{};
		//			VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation, &data));
		//			memcpy(data, meshes[0].vertex.data(), bufferCreateInfo.size);
		//			vmaUnmapMemory(m_device->GetVmaAllocator(), vertexBuffer->allocation);
		//		}
		//		{// index
		//			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * meshes[0].index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		//			VmaAllocationCreateInfo allocationCreateInfo{};
		//			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
		//			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &indexBuffer->buffer, &indexBuffer->allocation, &indexBuffer->allocationInfo));
		//			indexBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(indexBuffer->buffer);

		//			// mapping
		//			void* data{};
		//			VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation, &data));
		//			memcpy(data, meshes[0].index.data(), bufferCreateInfo.size);
		//			vmaUnmapMemory(m_device->GetVmaAllocator(), indexBuffer->allocation);
		//		}
		//		pushConstant.size = meshes[0].meshInfo.atlasSize;
		//		ASSERT(allocateLightmapAtlas(pushConstant.size, pushConstant.offset), "Failed to alloc from lightmap.");
		//		return true;
		//	}
		//}

		return false;
	}

	void LightmapBakingPipeline::Bake()
	{
		for (auto& buffer : m_vertexBuffer)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		}
		m_vertexBuffer.clear();
		for (auto& buffer : m_indexBuffer)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		}
		m_indexBuffer.clear();
		lightmapAllocateTools.current = glm::ivec2(0);
		lightmapAllocateTools.bottom = 0;
		m_isBaking = true;
	}

	void LightmapBakingPipeline::OutputLightmap(const RenderResources_ptr& resources)
	{
		if (m_isBaking)
		{
			resources->SaveImage(resources->GetLightmap(), "Lightmap.png", pushConstant.atlasSize);
			m_isBaking = false;
		}
	}

	void LightmapBakingPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// VS
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				0,
				static_cast<uint32_t>(sizeof(pushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("Lightmap.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("Lightmap.frag.spv")),
		};

		// pipelineVertexInputStateCreateInfo
		Utility::Vulkan::CreateInfo::VertexAttributeBits vertexAttributeBits =
			Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC3 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV0 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV1 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;

		// pipelineVertexInputStateCreateInfo
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription = {
			Utility::Vulkan::CreateInfo::vertexInputBindingDescription(0, vertexAttributeBits)
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptiones = Utility::Vulkan::CreateInfo::vertexInputAttributeDescriptiones(0, vertexAttributeBits);
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptiones);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VkCompareOp::VK_COMPARE_OP_NEVER);
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
		};
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo(colorBlendAttachmentStates);
		std::vector<VkDynamicState> dynamicStates = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDynamicStateCreateInfo(dynamicStates);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = Utility::Vulkan::CreateInfo::graphicsPipelineCreateInfo(
			shaderStageCreateInfo,									// シェーダーステージ
			&pipelineVertexInputStateCreateInfo,					// 頂点入力
			&inputAssemblyStateCreateInfo,							// 入力アセンブリ
			&viewportStateCreateInfo,								// ビューポートステート
			&rasterizationStateCreateInfo,							// ラスタライゼーション
			&multisampleStateCreateInfo,							// マルチサンプリング
			&depthStencilStateCreateInfo,							// 深度/ステンシル
			&colorBlendStateCreateInfo,								// カラーブレンディング
			&dynamicStateCreateInfo,								// 動的状態
			m_pipelineLayout,										// パイプラインレイアウト
			renderPass);											// レンダーパス
		graphicsPipelineCreateInfo.subpass = 0;
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}

	bool LightmapBakingPipeline::allocateLightmapAtlas(const glm::ivec2& size, glm::ivec2& offset)
	{
		// 間隔
		const glm::ivec2 padding = glm::ivec2(2);

		// 詰め込むサイズ
		uint32_t w = size.x + padding.x;
		uint32_t h = size.y + padding.y;

		// 一枚のアトラスに収まらない
		if ((w > pushConstant.atlasSize.x) || (h > pushConstant.atlasSize.y)) return false;

		// 横が足りない
		if (lightmapAllocateTools.current.x + w > pushConstant.atlasSize.x)
		{
			// 改行
			lightmapAllocateTools.current.x = 0;
			lightmapAllocateTools.current.y += lightmapAllocateTools.bottom;
			lightmapAllocateTools.bottom = 0;
		}

		// 縦が足りない
		if (lightmapAllocateTools.current.y + h > pushConstant.atlasSize.y) return false;

		// 詰め込み成功!!
		offset = lightmapAllocateTools.current + padding;

		// 次の詰め込み位置を更新
		lightmapAllocateTools.current.x += w;
		lightmapAllocateTools.bottom = std::max(lightmapAllocateTools.bottom, h);

		return true;
	}
}