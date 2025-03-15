// Copyright (c) 2025 kong9812
#include "RenderResources.h"
#include "VK_Loader.h"

namespace MyosotisFW::System::Render
{
	RenderResources::RenderResources(const RenderDevice_ptr& device, const uint32_t width, const uint32_t height)
	{
		m_device = device;

		prepareAttachments(width, height);
	}

	RenderResources::~RenderResources()
	{
		{// attachment
			vmaDestroyImage(m_device->GetVmaAllocator(), m_position.image, m_position.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_normal.image, m_normal.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_baseColor.image, m_baseColor.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_shadowMap.image, m_shadowMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_lightingResult.image, m_lightingResult.allocation);
			vkDestroyImageView(*m_device, m_position.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_normal.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_baseColor.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_shadowMap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_lightingResult.view, m_device->GetAllocationCallbacks());

			vkDestroyImage(*m_device, m_depthStencil.image, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_depthStencil.view, m_device->GetAllocationCallbacks());
			vkFreeMemory(*m_device, m_depthStencil.memory, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, VkShaderModule> shaderMoudle : m_shaderModules)
		{
			vkDestroyShaderModule(*m_device, shaderMoudle.second, m_device->GetAllocationCallbacks());
		}
		m_shaderModules.clear();

		m_meshVertexDatas.clear();

		for (std::pair<std::string, VMAImage> image : m_images)
		{
			vmaDestroyImage(m_device->GetVmaAllocator(), image.second.image, image.second.allocation);
			vkDestroyImageView(*m_device, image.second.view, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, VMAImage> image : m_cubeImages)
		{
			vmaDestroyImage(m_device->GetVmaAllocator(), image.second.image, image.second.allocation);
			vkDestroyImageView(*m_device, image.second.view, m_device->GetAllocationCallbacks());
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

	std::vector<Mesh> RenderResources::GetMeshVertex(const std::string& fileName)
	{
		auto vertexData = m_meshVertexDatas.find(fileName);
		if (vertexData == m_meshVertexDatas.end())
		{
			// ないなら読み込む
			m_meshVertexDatas.emplace(fileName, Utility::Loader::loadFbx(fileName));
		}
		return m_meshVertexDatas[fileName];
	}

	ImageWithSampler RenderResources::GetImage(const std::string& fileName)
	{
		auto image = m_images.find(fileName);
		if (image == m_images.end())
		{
			// todo.　実装場所を変える
			VkCommandPoolCreateInfo commandPoolCreateInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetTransferFamilyIndex());
			VkCommandPool pool{};
			VkQueue queue{};
			vkGetDeviceQueue(*m_device, m_device->GetTransferFamilyIndex(), 0, &queue);
			VK_VALIDATION(vkCreateCommandPool(*m_device, &commandPoolCreateInfo, m_device->GetAllocationCallbacks(), &pool));

			// ないなら読み込む
			m_images.emplace(fileName, Utility::Loader::loadImage(
				*m_device,
				queue,
				pool,
				m_device->GetVmaAllocator(),
				fileName,
				m_device->GetAllocationCallbacks()));

			// 後片付け
			vkDestroyCommandPool(*m_device, pool, m_device->GetAllocationCallbacks());
		}
		return { m_images[fileName].image,  m_images[fileName].view };
	}

	ImageWithSampler RenderResources::GetCubeImage(const std::vector<std::string>& fileNames)
	{
		auto image = m_cubeImages.find(fileNames[0]);
		if (image == m_cubeImages.end())
		{
			// todo.　実装場所を変える
			VkCommandPoolCreateInfo commandPoolCreateInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetTransferFamilyIndex());
			VkCommandPool pool{};
			VkQueue queue{};
			vkGetDeviceQueue(*m_device, m_device->GetTransferFamilyIndex(), 0, &queue);
			VK_VALIDATION(vkCreateCommandPool(*m_device, &commandPoolCreateInfo, m_device->GetAllocationCallbacks(), &pool));

			// ないなら読み込む
			m_cubeImages.emplace(fileNames[0], Utility::Loader::loadCubeImage(
				*m_device,
				queue,
				pool,
				m_device->GetVmaAllocator(),
				fileNames,
				m_device->GetAllocationCallbacks()));

			// 後片付け
			vkDestroyCommandPool(*m_device, pool, m_device->GetAllocationCallbacks());
		}
		return { m_cubeImages[fileNames[0]].image,  m_cubeImages[fileNames[0]].view };
	}

	void RenderResources::prepareAttachments(const uint32_t width, const uint32_t height)
	{
		{// StaticMesh pass
			// image
			VkImageCreateInfo imageCreateInfoForDepthStencil = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthFormat, width, height);
			VK_VALIDATION(vkCreateImage(*m_device, &imageCreateInfoForDepthStencil, m_device->GetAllocationCallbacks(), &m_depthStencil.image));
			// allocate
			m_device->ImageMemoryAllocate(m_depthStencil);
			// image view
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
		}

		{// attachments position
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredPositionFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_position.image, &m_position.allocation, &m_position.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_position.image, AppInfo::g_deferredPositionFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_position.view));
		}
		{// attachments normal
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_deferredNormalFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_normal.image, &m_normal.allocation, &m_normal.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_normal.image, AppInfo::g_deferredNormalFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_normal.view));
		}
		{// attachments base color
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_colorFormat, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_baseColor.image, &m_baseColor.allocation, &m_baseColor.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_baseColor.image, AppInfo::g_colorFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_baseColor.view));
		}
		{// attachments lighting result
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_lightingResult.image, &m_lightingResult.allocation, &m_lightingResult.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_lightingResult.image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_lightingResult.view));
		}
	}
}