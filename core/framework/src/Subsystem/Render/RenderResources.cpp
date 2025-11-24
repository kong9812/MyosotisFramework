// Copyright (c) 2025 kong9812
#include "RenderResources.h"
#include "RenderDevice.h"
#include "VK_Loader.h"
#include "RenderQueue.h"
#include "MeshInfoDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	RenderResources::~RenderResources()
	{
		{// attachment
			vkDestroySampler(*m_device, m_shadowMap.sampler, m_device->GetAllocationCallbacks());
			vkDestroySampler(*m_device, m_hiZDepthMap.sampler, m_device->GetAllocationCallbacks());
			vkDestroySampler(*m_device, m_primaryDepthStencil.sampler, m_device->GetAllocationCallbacks());

			vmaDestroyImage(m_device->GetVmaAllocator(), m_depthStencil.image, m_depthStencil.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_position.image, m_position.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_normal.image, m_normal.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_baseColor.image, m_baseColor.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_shadowMap.image, m_shadowMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_lightingResult.image, m_lightingResult.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_mainRenderTarget.image, m_mainRenderTarget.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_idMap.image, m_idMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_hiZDepthMap.image, m_hiZDepthMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_primaryDepthStencil.image, m_primaryDepthStencil.allocation);

			vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_position.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_normal.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_baseColor.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_shadowMap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_lightingResult.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_mainRenderTarget.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_idMap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_hiZDepthMap.view, m_device->GetAllocationCallbacks());
			for (VkImageView& view : m_hiZDepthMap.mipView)
				vkDestroyImageView(*m_device, view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_primaryDepthStencil.view, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, VkShaderModule> shaderMoudle : m_shaderModules)
		{
			vkDestroyShaderModule(*m_device, shaderMoudle.second, m_device->GetAllocationCallbacks());
		}
		m_shaderModules.clear();

		m_meshVertexData.clear();

		for (std::pair<std::string, Image> image : m_images)
		{
			vmaDestroyImage(m_device->GetVmaAllocator(), image.second.image, image.second.allocation);
			vkDestroyImageView(*m_device, image.second.view, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, Image> image : m_cubeImages)
		{
			vmaDestroyImage(m_device->GetVmaAllocator(), image.second.image, image.second.allocation);
			vkDestroyImageView(*m_device, image.second.view, m_device->GetAllocationCallbacks());
		}
	}

	void RenderResources::Initialize(const uint32_t width, const uint32_t height)
	{
		{// depth/stencil
			VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfoForDepthStencil, &allocationCreateInfo, &m_depthStencil.image, &m_depthStencil.allocation, &m_depthStencil.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_depthStencil.image, AppInfo::g_depthFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.view));
		}
		{// shadow map
			VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_shadowMapFormat, AppInfo::g_shadowMapSize, AppInfo::g_shadowMapSize);
			imageCreateInfoForDepthStencil.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfoForDepthStencil, &allocationCreateInfo, &m_shadowMap.image, &m_shadowMap.allocation, &m_shadowMap.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepthStencil(m_shadowMap.image, AppInfo::g_shadowMapFormat);
			imageViewCreateInfoForDepthStencil.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_shadowMap.view));
			VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
			VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_shadowMap.sampler));
		}
		{// position
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredPositionFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_position.image, &m_position.allocation, &m_position.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_position.image, AppInfo::g_deferredPositionFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_position.view));
		}
		{// normal
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredNormalFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_normal.image, &m_normal.allocation, &m_normal.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_normal.image, AppInfo::g_deferredNormalFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_normal.view));
		}
		{// base color
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_colorFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_baseColor.image, &m_baseColor.allocation, &m_baseColor.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_baseColor.image, AppInfo::g_colorFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_baseColor.view));
		}
		{// lighting result
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_lightingResult.image, &m_lightingResult.allocation, &m_lightingResult.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_lightingResult.image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_lightingResult.view));
		}
		{// main render target
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_mainRenderTarget.image, &m_mainRenderTarget.allocation, &m_mainRenderTarget.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_mainRenderTarget.image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_mainRenderTarget.view));
		}
		{// ID Map
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_idMapFormat, width, height);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_idMap.image, &m_idMap.allocation, &m_idMap.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_idMap.image, AppInfo::g_idMapFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_idMap.view));
		}

		{// Hi-Z DepthMap (MipLevels: 3)
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForHiZDepthStencil(AppInfo::g_hiZDepthFormat, width, height, AppInfo::g_hiZMipLevels);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT |
				VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_hiZDepthMap.image, &m_hiZDepthMap.allocation, &m_hiZDepthMap.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_hiZDepthMap.image, AppInfo::g_hiZDepthFormat);
			imageViewCreateInfo.subresourceRange.levelCount = AppInfo::g_hiZMipLevels;
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap.view));
			VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
			VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap.sampler));
			m_hiZDepthMap.mipView.resize(AppInfo::g_hiZMipLevels);
			for (uint8_t i = 0; i < AppInfo::g_hiZMipLevels; i++)
			{
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_hiZDepthMap.image, AppInfo::g_hiZDepthFormat);
				imageViewCreateInfo.subresourceRange.baseMipLevel = i;
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap.mipView[i]));
			}
		}

		{// Primary Depth Map
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_primaryDepthFormat, width, height);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_primaryDepthStencil.image, &m_primaryDepthStencil.allocation, &m_primaryDepthStencil.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_primaryDepthStencil.image, AppInfo::g_primaryDepthFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_primaryDepthStencil.view));
			VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
			VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_primaryDepthStencil.sampler));
		}
	}

	VkShaderModule RenderResources::GetShaderModules(const std::string& fileName)
	{
		auto shaderModule = m_shaderModules.find(fileName);
		if (shaderModule == m_shaderModules.end())
		{
			// ないなら読み込む
			m_shaderModules.emplace(fileName, Utility::Loader::loadShader(*m_device, fileName, m_device->GetAllocationCallbacks()));
		}
		return m_shaderModules[fileName];
	}

	std::vector<Mesh> RenderResources::GetMesh(const std::string& fileName)
	{
		auto vertexData = m_meshVertexData.find(fileName);
		if (vertexData == m_meshVertexData.end())
		{
			// 拡張子判定
			std::string extension = std::filesystem::path(fileName).extension().string();
			if ((extension == ".fbx") || (extension == ".FBX"))
			{
				m_meshVertexData.emplace(fileName, Utility::Loader::loadFbx(fileName));
			}
			else if ((extension == ".gltf") || (extension == ".GLTF"))
			{
				m_meshVertexData.emplace(fileName, Utility::Loader::loadGltf(fileName));
			}
			else
			{
				ASSERT(false, "Unsupported mesh file format: " + fileName);
			}
		}
		return m_meshVertexData[fileName];
	}

	Image RenderResources::GetImage(const std::string& fileName)
	{
		auto image = m_images.find(fileName);
		if (image == m_images.end())
		{
			// todo.　実装場所を変える
			VkCommandPool pool{};
			RenderQueue_ptr transferQueue = m_device->GetTransferQueue();
			VkCommandPoolCreateInfo commandPoolCreateInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(transferQueue->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &commandPoolCreateInfo, m_device->GetAllocationCallbacks(), &pool));

			// ないなら読み込む
			m_images.emplace(fileName, Utility::Loader::loadImage(
				*m_device,
				transferQueue,
				pool,
				m_device->GetVmaAllocator(),
				fileName,
				m_device->GetAllocationCallbacks()));

			// 後片付け
			vkDestroyCommandPool(*m_device, pool, m_device->GetAllocationCallbacks());
		}
		return { m_images[fileName].image,  m_images[fileName].view };
	}

	Image RenderResources::GetCubeImage(const std::vector<std::string>& fileNames)
	{
		auto image = m_cubeImages.find(fileNames[0]);
		if (image == m_cubeImages.end())
		{
			// todo.　実装場所を変える
			VkCommandPool pool{};
			RenderQueue_ptr transferQueue = m_device->GetTransferQueue();
			VkCommandPoolCreateInfo commandPoolCreateInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(transferQueue->GetQueueFamilyIndex());
			VK_VALIDATION(vkCreateCommandPool(*m_device, &commandPoolCreateInfo, m_device->GetAllocationCallbacks(), &pool));

			// ないなら読み込む
			m_cubeImages.emplace(fileNames[0], Utility::Loader::loadCubeImage(
				*m_device,
				transferQueue,
				pool,
				m_device->GetVmaAllocator(),
				fileNames,
				m_device->GetAllocationCallbacks()));

			// 後片付け
			vkDestroyCommandPool(*m_device, pool, m_device->GetAllocationCallbacks());
		}
		return { m_cubeImages[fileNames[0]].image,  m_cubeImages[fileNames[0]].view };
	}

	void RenderResources::Resize(const uint32_t width, const uint32_t height)
	{
		{// attachment
			vmaDestroyImage(m_device->GetVmaAllocator(), m_depthStencil.image, m_depthStencil.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_position.image, m_position.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_normal.image, m_normal.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_baseColor.image, m_baseColor.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_shadowMap.image, m_shadowMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_lightingResult.image, m_lightingResult.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_mainRenderTarget.image, m_mainRenderTarget.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_idMap.image, m_idMap.allocation);

			vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_position.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_normal.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_baseColor.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_shadowMap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_lightingResult.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_mainRenderTarget.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_idMap.view, m_device->GetAllocationCallbacks());
		}
		Initialize(width, height);
	}
}