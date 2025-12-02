// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

#include "istb_image.h"
#include "AppInfo.h"

#include "Image.h"
#include "Buffer.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "RenderQueue.h"

namespace Utility::Loader {
	inline MyosotisFW::Image loadCubeImage(VkDevice device, MyosotisFW::System::Render::RenderQueue_ptr queue, VkCommandPool commandPool, VmaAllocator allocator, std::vector<std::string> fileNames, const VkAllocationCallbacks* pAllocationCallbacks = nullptr)
	{
		MyosotisFW::Image image{};

		// loadCubeImage fence
		VkFence fence = VK_NULL_HANDLE;
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(device, &fenceCreateInfo, pAllocationCallbacks, &fence));

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

			queue->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

			// clean up
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
			vkDestroyFence(device, fence, pAllocationCallbacks);
		}
		return image;
	}
}
