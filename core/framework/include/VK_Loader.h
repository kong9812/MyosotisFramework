// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
#include <chrono>

#include "iRapidJson.h"
#include "iofbx.h"
#include "istb_image.h"

#include "Logger.h"
#include "AppInfo.h"
#include "Structs.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "RenderQueue.h"

namespace Utility::Loader {
	inline VkShaderModule loadShader(VkDevice device, std::string fileName, const VkAllocationCallbacks* pAllocator = nullptr)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_shaderFolder + fileName);

		std::ifstream file(absolutePath.string().c_str(), std::ios::ate | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open shader file: " + std::string(MyosotisFW::AppInfo::g_shaderFolder) + fileName);

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buf(fileSize);
		file.seekg(0);
		file.read(buf.data(), fileSize);
		file.close();

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = buf.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buf.data());

		VkShaderModule shaderModule{};
		VK_VALIDATION(vkCreateShaderModule(device, &shaderModuleCreateInfo, pAllocator, &shaderModule));
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return shaderModule;
	}

	inline rapidjson::Document loadGameStageFile(std::string fileName)
	{
		std::ifstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		rapidjson::IStreamWrapper istream(file);
		rapidjson::Document doc{};
		doc.ParseStream(istream);
		file.close();
		return doc;
	}
	inline void saveGameStageFile(std::string fileName, rapidjson::Document& doc)
	{
		rapidjson::StringBuffer buffer{};
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName, std::ios::trunc);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		file << buffer.GetString();
		file.close();
	}

	inline ofbx::u32 triangulate(const ofbx::GeometryData& geom, const ofbx::GeometryPartition::Polygon& polygon, std::vector<uint32_t>& tri_indices) {
		// 必要な三角形数を計算
		ofbx::u32 triangleCount = polygon.vertex_count - 2;
		if (triangleCount <= 0) return 0;

		// tri_indices のサイズを確保
		tri_indices.reserve(tri_indices.size() + 3 * triangleCount);

		if (polygon.vertex_count == 3) {
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 1));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));
		}
		else if (polygon.vertex_count == 4) {
			// 四角形を三角形に分割
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 0));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 1));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));

			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 0));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 3));
		}
		else {
			// 多角形をトライアングルファンで分割
			for (ofbx::u32 tri = 0; tri < triangleCount; ++tri) {
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex));
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex) + 1 + tri);
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex) + 2 + tri);
			}
		}
		return 3 * triangleCount;
	}

	inline std::vector<MyosotisFW::Mesh> loadFbx(std::string fileName)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif

		std::ifstream file(MyosotisFW::AppInfo::g_modelFolder + fileName, std::ios::ate | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open fbx file: " + std::string(MyosotisFW::AppInfo::g_modelFolder) + fileName);
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buf(fileSize);
		file.seekg(0);
		file.read(buf.data(), fileSize);
		file.close();

		ofbx::IScene* scene = ofbx::load(
			reinterpret_cast<ofbx::u8*>(buf.data()),
			static_cast<ofbx::usize>(fileSize),
			static_cast<ofbx::u16>(ofbx::LoadFlags::NONE));

		std::vector<MyosotisFW::Mesh> meshes{};

		uint32_t indicesOffset = 0;
		uint32_t meshCount = scene->getMeshCount();
		std::vector<int> testList{};
		for (uint32_t meshIdx = 0; meshIdx < meshCount; meshIdx++)
		{
			MyosotisFW::Mesh meshData{};
			bool firstDataForAABB = true;
			const ofbx::Mesh* mesh = scene->getMesh(meshIdx);
			const ofbx::GeometryData& geomData = mesh->getGeometryData();
			const ofbx::Vec3Attributes positions = geomData.getPositions();
			for (uint32_t partitionIdx = 0; partitionIdx < geomData.getPartitionCount(); partitionIdx++)
			{
				const ofbx::GeometryPartition& geometryPartition = geomData.getPartition(partitionIdx);
				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];

					for (uint32_t vertexIdx = polygon.from_vertex; vertexIdx < polygon.from_vertex + polygon.vertex_count; vertexIdx++)
					{
						ofbx::Vec3 v = positions.get(vertexIdx);
						meshData.vertex.push_back(v.x);
						meshData.vertex.push_back(v.y);
						meshData.vertex.push_back(v.z);
						meshData.vertex.push_back(1.0f);

						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);

						// aabb
						if (firstDataForAABB)
						{
							meshData.min.x = v.x;
							meshData.min.y = v.y;
							meshData.min.z = v.z;
							meshData.max.x = v.x;
							meshData.max.y = v.y;
							meshData.max.z = v.z;
						}
						else
						{
							meshData.min.x = meshData.min.x < v.x ? meshData.min.x : v.x;
							meshData.min.y = meshData.min.y < v.y ? meshData.min.y : v.y;
							meshData.min.z = meshData.min.z < v.z ? meshData.min.z : v.z;
							meshData.max.x = meshData.max.x > v.x ? meshData.max.x : v.x;
							meshData.max.y = meshData.max.y > v.y ? meshData.max.y : v.y;
							meshData.max.z = meshData.max.z > v.z ? meshData.max.z : v.z;
						}
					}
				}

				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];
					triangulate(geomData, polygon, meshData.index);
				}
				indicesOffset += positions.count;
			}
			meshes.push_back(meshData);
		}
		scene->destroy();
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return meshes;
	}

	inline MyosotisFW::VMAImage loadImage(VkDevice device, MyosotisFW::System::Render::RenderQueue_ptr queue, VkCommandPool commandPool, VmaAllocator allocator, std::string fileName, const VkAllocationCallbacks* pAllocationCallbacks = nullptr)
	{
		MyosotisFW::VMAImage image{};

		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_textureFolder + fileName);

		int textureWidth = -1;
		int textureHeight = -1;
		int textureChannels = -1;
		stbi_uc* pixels = stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		ASSERT(pixels, "Failed to load image: " + absolutePath.string());
		size_t imageSize = textureWidth * textureHeight * textureChannels;

		{// GPU image
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfo(VkFormat::VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			VK_VALIDATION(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfo(image.image, VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
			VK_VALIDATION(vkCreateImageView(device, &imageViewCreateInfo, pAllocationCallbacks, &image.view));
		}

		MyosotisFW::Buffer stagingBuffer{};
		{// CPU buffer (staging buffer)
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(imageSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		}

		{// CPU -> GPU
			VkCommandBuffer commandBuffer{};
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

			void* data{};
			VK_VALIDATION(vmaMapMemory(allocator, stagingBuffer.allocation, &data));
			memcpy(data, pixels, imageSize);
			vmaUnmapMemory(allocator, stagingBuffer.allocation);
			stbi_image_free(pixels);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = image.image;
			imageMemoryBarrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VkBufferImageCopy bufferImageCopy = Utility::Vulkan::CreateInfo::bufferImageCopy(textureWidth, textureHeight);
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, image.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			queue->Submit(submitInfo);
			queue->WaitIdle();

			// clean up
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
		}
		return image;
	}

	inline MyosotisFW::VMAImage loadCubeImage(VkDevice device, MyosotisFW::System::Render::RenderQueue_ptr queue, VkCommandPool commandPool, VmaAllocator allocator, std::vector<std::string> fileNames, const VkAllocationCallbacks* pAllocationCallbacks = nullptr)
	{
		MyosotisFW::VMAImage image{};

		int textureWidth = -1;
		int textureHeight = -1;
		int textureChannels = -1;
		size_t imageSize = 0;
		std::vector<stbi_uc*> pixels{};
		for (uint32_t i = 0; i < fileNames.size(); i++)
		{
			std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_textureFolder + fileNames[i]);
			pixels.push_back(stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha));
			ASSERT(pixels[i], "Failed to load image: " + absolutePath.string());
			imageSize += textureWidth * textureHeight * textureChannels;
		}

		{// GPU image
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfo(VkFormat::VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight);
			imageCreateInfo.arrayLayers = 6;
			imageCreateInfo.flags = VkImageCreateFlagBits::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			VK_VALIDATION(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfo(image.image, VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
			imageViewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
			imageViewCreateInfo.subresourceRange.layerCount = 6;
			VK_VALIDATION(vkCreateImageView(device, &imageViewCreateInfo, pAllocationCallbacks, &image.view));
		}

		MyosotisFW::Buffer stagingBuffer{};
		{// CPU buffer (staging buffer)
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(imageSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		}

		{// CPU -> GPU
			VkCommandBuffer commandBuffer{};
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

			void* data{};
			VK_VALIDATION(vmaMapMemory(allocator, stagingBuffer.allocation, &data));
			size_t offset = 0;
			for (size_t i = 0; i < pixels.size(); i++)
			{
				memcpy(static_cast<uint8_t*>(data) + offset, pixels[i], textureWidth * textureHeight * textureChannels);
				offset += textureWidth * textureHeight * textureChannels;
				stbi_image_free(pixels[i]);
			}
			vmaUnmapMemory(allocator, stagingBuffer.allocation);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = image.image;
			imageMemoryBarrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 6;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			std::vector<VkBufferImageCopy> bufferImageCopy(pixels.size());
			for (size_t i = 0; i < pixels.size(); i++)
			{
				bufferImageCopy[i] = Utility::Vulkan::CreateInfo::bufferImageCopy(textureWidth, textureHeight);
				bufferImageCopy[i].bufferOffset = textureWidth * textureHeight * textureChannels * i;
				bufferImageCopy[i].imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
			}
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, image.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferImageCopy.size()), bufferImageCopy.data());

			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			queue->Submit(submitInfo);
			queue->WaitIdle();

			// clean up
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
		}
		return image;
	}
}
