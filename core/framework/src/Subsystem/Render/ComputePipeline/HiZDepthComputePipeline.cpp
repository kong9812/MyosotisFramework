// Copyright (c) 2025 kong9812
#pragma once
#include "HiZDepthComputePipeline.h"
#include "RenderDevice.h"
#include "RenderDescriptors.h"
#include "RenderResources.h"
#include "AppInfo.h"

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
					0, static_cast<uint32_t>(sizeof(depthCopyPushConstant))),
			};
			// [pipeline]layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_descriptors->GetBindlessMainDescriptorSetLayout() };
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
					0, static_cast<uint32_t>(sizeof(depthDownsamplePushConstant))),
			};
			// [pipeline]layout
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_descriptors->GetBindlessMainDescriptorSetLayout() };
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
	}

	void HiZDepthComputePipeline::Dispatch(const VkCommandBuffer& commandBuffer, const glm::vec2& screenSize)
	{
		uint32_t hiZDepthMipMapImageIndex[AppInfo::g_hiZMipLevels]{};
		uint32_t hiZDepthMapMipSamplerIndex[AppInfo::g_hiZMipLevels]{};
		for (uint8_t i = 0; i < AppInfo::g_hiZMipLevels; i++)
		{
			// [storage image] Set image layout for HiZ depth map
			VkDescriptorImageInfo hiZDepthMapDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, m_resources->GetHiZDepthMap().view[i], VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
			hiZDepthMipMapImageIndex[i] = m_descriptors->AddStorageImageInfo(hiZDepthMapDescriptorImageInfo);

			// [sampler] Set image layout for HiZ depth map
			VkDescriptorImageInfo hiZDepthMapSamplerDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(m_resources->GetHiZDepthMap().sampler[i], m_resources->GetHiZDepthMap().view[i], VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			hiZDepthMapMipSamplerIndex[i] = m_descriptors->AddCombinedImageSamplerInfo(hiZDepthMapSamplerDescriptorImageInfo);
		}

		// [sampler] Set image layout for primary depth/stencil map
		VkDescriptorImageInfo GetPrimaryDepthStencilDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(m_resources->GetPrimaryDepthStencil().sampler, m_resources->GetPrimaryDepthStencil().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		depthCopyPushConstant.primaryDepthSamplerID = m_descriptors->AddCombinedImageSamplerInfo(GetPrimaryDepthStencilDescriptorImageInfo);

		{// HiZ Depth Copy
			// Bind the compute pipeline
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthCopyShaderBase.pipeline);
			// Bind the descriptor sets
			std::vector<VkDescriptorSet> descriptorSets = { m_descriptors->GetBindlessMainDescriptorSet() };
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthCopyShaderBase.pipelineLayout, 0,
				static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
			// Bind the push constants
			depthCopyPushConstant.hiZImageID = hiZDepthMipMapImageIndex[0];
			depthCopyPushConstant.desSize = static_cast<glm::ivec2>(screenSize);
			vkCmdPushConstants(commandBuffer, m_hiZDepthCopyShaderBase.pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
				0, static_cast<uint32_t>(sizeof(depthCopyPushConstant)), &depthCopyPushConstant);
			// Dispatch the compute shader
			uint32_t threadNumX = 8;	// xのスレッド数
			uint32_t threadNumY = 8;	// yのスレッド数
			uint32_t groupX = (screenSize.x + (threadNumY - 1)) / threadNumX;
			uint32_t groupY = (screenSize.y + (threadNumY - 1)) / threadNumY;
			vkCmdDispatch(commandBuffer, groupX, groupY, 1);
		}
		{// HiZ Depth Downsample
			// Bind the compute pipeline
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthDownsampleShaderBase.pipeline);
			// Bind the descriptor sets
			std::vector<VkDescriptorSet> descriptorSets = { m_descriptors->GetBindlessMainDescriptorSet() };
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_hiZDepthDownsampleShaderBase.pipelineLayout, 0,
				static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

			uint32_t threadNumX = 16;	// xのスレッド数
			uint32_t threadNumY = 16;	// yのスレッド数
			uint32_t srcMip = 0;
			uint32_t dstMip = 1;
			glm::ivec2 mipSize = glm::ivec2(screenSize.x, screenSize.y);
			for (uint32_t i = 1; i < AppInfo::g_hiZMipLevels; i++, srcMip++, dstMip++)
			{
				// 次のmipサイズは半分
				mipSize.x = (mipSize.x + 1) / 2;
				mipSize.y = (mipSize.y + 1) / 2;

				// workgroup数計算（16で割って切り上げ）
				uint32_t groupX = (mipSize.x + threadNumX - 1) / threadNumX;
				uint32_t groupY = (mipSize.y + threadNumY - 1) / threadNumY;

				// Push constant 設定
				depthDownsamplePushConstant.hiZSamplerID = hiZDepthMapMipSamplerIndex[srcMip];
				depthDownsamplePushConstant.hiZImageID = hiZDepthMipMapImageIndex[dstMip];
				depthDownsamplePushConstant.desSize = mipSize;
				depthDownsamplePushConstant.srcMip = srcMip;
				vkCmdPushConstants(commandBuffer, m_hiZDepthDownsampleShaderBase.pipelineLayout,
					VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
					0, static_cast<uint32_t>(sizeof(depthDownsamplePushConstant)), &depthDownsamplePushConstant);

				// Dispatch the compute shader
				vkCmdDispatch(commandBuffer, groupX, groupY, 1);
			}
		}
	}
}