// Copyright (c) 2025 kong9812
#pragma once
#include "HiZDepthComputePipeline.h"
#include "RenderDevice.h"
#include "RenderQueue.h"
#include "RenderResources.h"
#include "AppInfo.h"

#include "SceneInfoDescriptorSet.h"
#include "ObjectInfoDescriptorSet.h"
#include "MeshInfoDescriptorSet.h"
#include "TextureDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	HiZDepthComputePipeline::~HiZDepthComputePipeline()
	{
		// Destroy pipelines and layouts
		vkDestroyPipelineLayout(*m_device, m_hiZDepthCopyShaderBase.pipelineLayout, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_hiZDepthCopyShaderBase.pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_hiZDepthDownsampleShaderBase.pipelineLayout, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_hiZDepthDownsampleShaderBase.pipeline, m_device->GetAllocationCallbacks());
	}

	void HiZDepthComputePipeline::Initialize()
	{
		{// HiZ Depth Copy Shader
			// push constant
			std::vector<VkPushConstantRange> pushConstantRange = {
				// CS
				Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
					0, static_cast<uint32_t>(sizeof(DepthCopyPushConstant))),
			};
			// [pipeline]layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
			pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
			VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthCopyShaderBase.pipelineLayout));
			// [pipeline]shader
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, m_resources->GetShaderModules("HiZDepthCopy.comp.spv"));
			// [pipeline]computePipeline
			VkComputePipelineCreateInfo computePipelineCreateInfo = Utility::Vulkan::CreateInfo::computePipelineCreateInfo(m_hiZDepthCopyShaderBase.pipelineLayout, shaderStageCreateInfo);
			VK_VALIDATION(vkCreateComputePipelines(*m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthCopyShaderBase.pipeline));
		}
		{// HiZ Depth Downsample Shader
			// push constant
			std::vector<VkPushConstantRange> pushConstantRange = {
				// CS
				Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
					0, static_cast<uint32_t>(sizeof(DepthDownsamplePushConstant))),
			};
			// [pipeline]layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
			pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
			VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthDownsampleShaderBase.pipelineLayout));
			// [pipeline]shader
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, m_resources->GetShaderModules("HiZDepthDownsample.comp.spv"));
			// [pipeline]computePipeline
			VkComputePipelineCreateInfo computePipelineCreateInfo = Utility::Vulkan::CreateInfo::computePipelineCreateInfo(m_hiZDepthDownsampleShaderBase.pipelineLayout, shaderStageCreateInfo);
			VK_VALIDATION(vkCreateComputePipelines(*m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthDownsampleShaderBase.pipeline));
		}

		{// DepthBuffer -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			std::array<VkImageMemoryBarrier, AppInfo::g_maxInFlightFrameCount> barrier{};
			for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
			{
				// Barrier
				const Image& depthBuffer = m_resources->GetDepthBuffer(i);
				barrier[i].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier[i].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
				barrier[i].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
				barrier[i].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				barrier[i].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].image = depthBuffer.image;
				barrier[i].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);

				// forを便乗する
				{// PushConstant Data
					// Hi-Z Depth
					const Image& hiZDepthMap = m_resources->GetHiZDepthMap(i);
					uint32_t hiZMipLevels = static_cast<uint32_t>(hiZDepthMap.mipView.size());
					m_hiZDepthMipMapImageIndex[i].resize(hiZMipLevels);
					for (uint8_t j = 0; j < hiZMipLevels; j++)
					{
						// image mips (for write)
						VkDescriptorImageInfo hiZDepthMapDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, hiZDepthMap.mipView[j], VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
						m_hiZDepthMipMapImageIndex[i][j] = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::StorageImage, hiZDepthMapDescriptorImageInfo);
					}
					// image (for read)
					VkDescriptorImageInfo hiZDepthMapSamplerDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(hiZDepthMap.sampler, hiZDepthMap.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					depthDownsamplePushConstant[i].hiZSamplerID = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, hiZDepthMapSamplerDescriptorImageInfo);

					// Depth
					const Image& previousDepth = m_resources->GetDepthBuffer(i);
					VkDescriptorImageInfo depthBufferDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(previousDepth.sampler, previousDepth.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					m_depthBufferSamplerID[i] = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, depthBufferDescriptorImageInfo);
				}
			}

			VkFence fence = VK_NULL_HANDLE;
			VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

			VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barrier.size()), barrier.data());
			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

			m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}

		// デップスクリア (初期化)
		depthCopyPushConstant[0].depthClear = 1;	// 最初のだけでOK!
	}

	void HiZDepthComputePipeline::Dispatch(const VkCommandBuffer& commandBuffer, const uint32_t dstFrameIndex, const uint32_t srcFrameIndex, const glm::vec2& screenSize)
	{
		uint32_t hiZMipLevels = static_cast<uint32_t>(m_resources->GetHiZDepthMap(dstFrameIndex).mipView.size());

		depthCopyPushConstant[dstFrameIndex].depthBufferSamplerID = m_depthBufferSamplerID[srcFrameIndex];

		{// HiZ Depth Copy
			// Bind the compute pipeline
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthCopyShaderBase.pipeline);
			// Bind the descriptor sets
			std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthCopyShaderBase.pipelineLayout, 0,
				static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
			// Bind the push constants
			depthCopyPushConstant[dstFrameIndex].hiZImageID = m_hiZDepthMipMapImageIndex[dstFrameIndex][0];
			depthCopyPushConstant[dstFrameIndex].desSize = static_cast<glm::ivec2>(screenSize);
			vkCmdPushConstants(commandBuffer, m_hiZDepthCopyShaderBase.pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
				0, static_cast<uint32_t>(sizeof(DepthCopyPushConstant)), &depthCopyPushConstant[dstFrameIndex]);
			// Dispatch the compute shader
			uint32_t threadNumX = 16;	// xのスレッド数
			uint32_t threadNumY = 16;	// yのスレッド数
			uint32_t groupX = (screenSize.x + (threadNumY - 1)) / threadNumX;
			uint32_t groupY = (screenSize.y + (threadNumY - 1)) / threadNumY;
			vkCmdDispatch(commandBuffer, groupX, groupY, 1);
		}
		{// HiZ Depth Downsample
			uint32_t threadNumX = 16;	// xのスレッド数
			uint32_t threadNumY = 16;	// yのスレッド数
			uint32_t srcMip = 0;
			uint32_t dstMip = 1;
			glm::ivec2 mipSize = glm::ivec2(screenSize.x, screenSize.y);

			{// mip1を UNDEFINED->GENERALに遷移
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = m_resources->GetHiZDepthMap(dstFrameIndex).image;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				barrier.subresourceRange.baseMipLevel = 1;
				barrier.subresourceRange.levelCount = hiZMipLevels - 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier);
			}

			for (uint32_t i = 1; i < hiZMipLevels; i++, srcMip++, dstMip++)
			{
				// Bind the compute pipeline
				vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthDownsampleShaderBase.pipeline);
				// Bind the descriptor sets
				std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
				vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthDownsampleShaderBase.pipelineLayout, 0,
					static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

				mipSize.x = (mipSize.x + 1) / 2;
				mipSize.y = (mipSize.y + 1) / 2;

				uint32_t groupX = (mipSize.x + threadNumX - 1) / threadNumX;
				uint32_t groupY = (mipSize.y + threadNumY - 1) / threadNumY;

				// Push constant 設定
				depthDownsamplePushConstant[dstFrameIndex].hiZImageID = m_hiZDepthMipMapImageIndex[dstFrameIndex][dstMip];
				depthDownsamplePushConstant[dstFrameIndex].desSize = mipSize;
				depthDownsamplePushConstant[dstFrameIndex].srcMip = srcMip;
				vkCmdPushConstants(commandBuffer, m_hiZDepthDownsampleShaderBase.pipelineLayout,
					VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
					0, static_cast<uint32_t>(sizeof(DepthDownsamplePushConstant)), &depthDownsamplePushConstant);

				// Dispatch the compute shader
				vkCmdDispatch(commandBuffer, groupX, groupY, 1);

				// バリア設定: 書き込み完了後、同じGENERALレイアウトの中でシェーダ読み書きの同期だけ行う
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;    // 既にGENERALのはず
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;    // 同じレイアウト
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = m_resources->GetHiZDepthMap(dstFrameIndex).image;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				barrier.subresourceRange.baseMipLevel = dstMip;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			}
		}

		// 初期化完了
		depthCopyPushConstant[dstFrameIndex].depthClear = 0;
	}

	void HiZDepthComputePipeline::Resize()
	{
		{// DepthBuffer -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			std::array<VkImageMemoryBarrier, AppInfo::g_maxInFlightFrameCount> barrier{};
			for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
			{
				// Barrier
				const Image& depthBuffer = m_resources->GetDepthBuffer(i);
				barrier[i].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier[i].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
				barrier[i].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
				barrier[i].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				barrier[i].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].image = depthBuffer.image;
				barrier[i].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);

				// forを便乗する
				{// PushConstant Data
					// Hi-Z Depth
					const Image& hiZDepthMap = m_resources->GetHiZDepthMap(i);
					uint32_t newImageCount = static_cast<uint32_t>(hiZDepthMap.mipView.size());
					uint32_t oldImageCount = static_cast<uint32_t>(m_hiZDepthMipMapImageIndex[i].size());
					if (oldImageCount < newImageCount)
					{
						m_hiZDepthMipMapImageIndex[i].resize(newImageCount);
					}
					else if (oldImageCount > newImageCount)
					{
						const uint32_t deleteCount = oldImageCount - newImageCount;
						for (uint32_t j = newImageCount; j < oldImageCount; j++)
						{
							// delete (SetDummy)
							m_renderDescriptors->GetTextureDescriptorSet()->DeleteImage(m_hiZDepthMipMapImageIndex[i][j], TextureDescriptorSet::DescriptorBindingIndex::StorageImage);
						}
						m_hiZDepthMipMapImageIndex[i].resize(newImageCount);
					}
					for (uint32_t j = 0; j < static_cast<uint32_t>(m_hiZDepthMipMapImageIndex[i].size()); j++)
					{
						if (j < oldImageCount)
						{
							// update
							VkDescriptorImageInfo hiZDepthMapDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, hiZDepthMap.mipView[j], VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
							m_renderDescriptors->GetTextureDescriptorSet()->UpdateImage(m_hiZDepthMipMapImageIndex[i][j], TextureDescriptorSet::DescriptorBindingIndex::StorageImage, hiZDepthMapDescriptorImageInfo);
						}
						else if (j >= oldImageCount)
						{
							// add
							VkDescriptorImageInfo hiZDepthMapDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, hiZDepthMap.mipView[j], VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
							m_hiZDepthMipMapImageIndex[i][j] = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::StorageImage, hiZDepthMapDescriptorImageInfo);
						}
					}
					// image (for read)
					VkDescriptorImageInfo hiZDepthMapSamplerDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(hiZDepthMap.sampler, hiZDepthMap.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					m_renderDescriptors->GetTextureDescriptorSet()->UpdateImage(depthDownsamplePushConstant[i].hiZSamplerID, TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, hiZDepthMapSamplerDescriptorImageInfo);

					// Depth
					const Image& previousDepth = m_resources->GetDepthBuffer(i);
					VkDescriptorImageInfo depthBufferDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(previousDepth.sampler, previousDepth.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					m_renderDescriptors->GetTextureDescriptorSet()->UpdateImage(m_depthBufferSamplerID[i], TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, depthBufferDescriptorImageInfo);
				}
			}

			VkFence fence = VK_NULL_HANDLE;
			VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

			VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barrier.size()), barrier.data());
			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

			m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}

		// デップスクリア (初期化)
		depthCopyPushConstant[0].depthClear = 1;	// 最初のだけでOK!
	}
}