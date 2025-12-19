// Copyright (c) 2025 kong9812
#include "RenderResources.h"
#include "RenderDevice.h"
#include "RenderQueue.h"
#include "RenderDescriptors.h"

#include "ShaderIo.h"
#include "CubeImageIo.h"
#include "ImageIo.h"
#include "FBXIo.h"
#include "GLTFIo.h"
#include "MFModelIo.h"
#include "TerrainIo.h"

namespace MyosotisFW::System::Render
{
	RenderResources::~RenderResources()
	{
		{// attachment
			vkDestroySampler(*m_device, m_hiZDepthMap.sampler, m_device->GetAllocationCallbacks());
			vkDestroySampler(*m_device, m_primaryDepthStencil.sampler, m_device->GetAllocationCallbacks());

			vmaDestroyImage(m_device->GetVmaAllocator(), m_mainRenderTarget.image, m_mainRenderTarget.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_visibilityBuffer.image, m_visibilityBuffer.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_hiZDepthMap.image, m_hiZDepthMap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_primaryDepthStencil.image, m_primaryDepthStencil.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_lightmap.image, m_lightmap.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_rayTracingRenderTarget.image, m_rayTracingRenderTarget.allocation);

			vkDestroyImageView(*m_device, m_mainRenderTarget.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_visibilityBuffer.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_hiZDepthMap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_lightmap.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_rayTracingRenderTarget.view, m_device->GetAllocationCallbacks());
			for (VkImageView& view : m_hiZDepthMap.mipView)
				vkDestroyImageView(*m_device, view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_primaryDepthStencil.view, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, VkShaderModule> shaderMoudle : m_shaderModules)
		{
			vkDestroyShaderModule(*m_device, shaderMoudle.second, m_device->GetAllocationCallbacks());
		}
		m_shaderModules.clear();

		for (std::pair<std::string, std::vector<Mesh>> meshList : m_meshes)
		{
			for (Mesh& mesh : meshList.second)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh.indexBuffer.buffer, mesh.indexBuffer.allocation);
			}
		}
		m_meshes.clear();

		for (std::pair<std::string, std::vector<Mesh>> meshList : m_terrains)
		{
			for (Mesh& mesh : meshList.second)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh.indexBuffer.buffer, mesh.indexBuffer.allocation);
			}
		}
		m_terrains.clear();

		for (std::pair<Shape::PrimitiveGeometryShape, Mesh> meshPair : m_primitiveGeometryMeshes)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), meshPair.second.vertexBuffer.buffer, meshPair.second.vertexBuffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), meshPair.second.indexBuffer.buffer, meshPair.second.indexBuffer.allocation);
		}
		m_primitiveGeometryMeshes.clear();

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

		for (VkSampler& sampler : m_samplers)
		{
			vkDestroySampler(*m_device, sampler, m_device->GetAllocationCallbacks());
		}
	}

	void RenderResources::Initialize(const uint32_t width, const uint32_t height)
	{
		{// main render target
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, width, height);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_mainRenderTarget.image, &m_mainRenderTarget.allocation, &m_mainRenderTarget.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_mainRenderTarget.image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_mainRenderTarget.view));
		}
		{// Visibility Buffer
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_visibilityBufferFormat, width, height);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_visibilityBuffer.image, &m_visibilityBuffer.allocation, &m_visibilityBuffer.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_visibilityBuffer.image, AppInfo::g_visibilityBufferFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_visibilityBuffer.view));
		}

		{// Hi-Z DepthMap
			uint32_t hiZMipLevels = floor(log2(std::max(width, height)));
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForHiZDepthStencil(AppInfo::g_hiZDepthFormat, width, height, hiZMipLevels);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT |
				VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_hiZDepthMap.image, &m_hiZDepthMap.allocation, &m_hiZDepthMap.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_hiZDepthMap.image, AppInfo::g_hiZDepthFormat);
			imageViewCreateInfo.subresourceRange.levelCount = hiZMipLevels;
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap.view));
			VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
			VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap.sampler));
			m_hiZDepthMap.mipView.resize(hiZMipLevels);
			for (uint8_t i = 0; i < hiZMipLevels; i++)
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

		{// Lightmap
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_lightmapFormat, AppInfo::g_lightmapSize, AppInfo::g_lightmapSize);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_lightmap.image, &m_lightmap.allocation, &m_lightmap.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_lightmap.image, AppInfo::g_lightmapFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_lightmap.view));
		}

		{// ray tracing render target
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_rayTracingRenderTargetFormat, width, height);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_rayTracingRenderTarget.image, &m_rayTracingRenderTarget.allocation, &m_rayTracingRenderTarget.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_rayTracingRenderTarget.image, AppInfo::g_rayTracingRenderTargetFormat);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_rayTracingRenderTarget.view));
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
		auto vertexData = m_meshes.find(fileName);
		if (vertexData == m_meshes.end())
		{
			// 拡張子判定
			std::string extension = std::filesystem::path(fileName).extension().string();
			if ((extension == ".fbx") || (extension == ".FBX"))
			{
				std::vector<Mesh> meshes = Utility::Loader::loadFbx(fileName);
				uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size() + m_terrains.size();
				for (Mesh& mesh : meshes)
				{
					mesh.meshInfo.meshID = meshID;
					meshID++;
				}
				createVertexIndexBuffer(meshes);
				m_onLoadedMesh(meshes);
				m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(fileName, meshes);
				m_meshes.emplace(fileName, meshes);
			}
			else if ((extension == ".gltf") || (extension == ".GLTF"))
			{
				std::vector<Mesh> meshes = Utility::Loader::loadGltf(fileName);
				uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size() + m_terrains.size();
				for (Mesh& mesh : meshes)
				{
					mesh.meshInfo.meshID = meshID;
					meshID++;
				}
				createVertexIndexBuffer(meshes);
				m_onLoadedMesh(meshes);
				m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(fileName, meshes);
				m_meshes.emplace(fileName, meshes);
			}
			else if ((extension == ".mfmodel") || (extension == ".MFMODEL"))
			{
				std::vector<Mesh> meshes = Utility::Loader::loadMFModel(fileName);
				uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size() + m_terrains.size();
				for (Mesh& mesh : meshes)
				{
					mesh.meshInfo.meshID = meshID;
					meshID++;
				}
				createVertexIndexBuffer(meshes);
				m_onLoadedMesh(meshes);
				m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(fileName, meshes);
				m_meshes.emplace(fileName, meshes);
			}
			else
			{
				ASSERT(false, "Unsupported mesh file format: " + fileName);
			}
		}
		return m_meshes[fileName];
	}

	std::vector<Mesh> RenderResources::GetTerrainMesh(const std::string& fileName)
	{
		auto vertexData = m_terrains.find(fileName);
		if (vertexData == m_terrains.end())
		{
			// 拡張子判定
			std::string extension = std::filesystem::path(fileName).extension().string();
			if ((extension == ".png") || (extension == ".PNG"))
			{
				std::vector<Mesh> meshes = Utility::Loader::loadTerrainMesh(fileName);
				uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size() + m_terrains.size();
				for (Mesh& mesh : meshes)
				{
					mesh.meshInfo.meshID = meshID;
					meshID++;
				}
				createVertexIndexBuffer(meshes);
				m_onLoadedMesh(meshes);
				m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(fileName, meshes);
				m_terrains.emplace(fileName, meshes);
			}
			else
			{
				ASSERT(false, "Unsupported terrain format: " + fileName);
			}
		}
		return m_terrains[fileName];
	}

	Mesh RenderResources::GetPrimitiveGeometryMesh(const Shape::PrimitiveGeometryShape shape)
	{
		auto vertexData = m_primitiveGeometryMeshes.find(shape);
		if (vertexData == m_primitiveGeometryMeshes.end())
		{
			std::vector<Mesh> meshes{ Shape::createShape(shape) };
			uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size() + m_terrains.size();
			for (Mesh& mesh : meshes)
			{
				mesh.meshInfo.meshID = meshID;
				meshID++;
			}
			createVertexIndexBuffer(meshes);
			m_renderDescriptors->GetMeshInfoDescriptorSet()->AddPrimitiveGeometry(shape, meshes[0]);
			m_primitiveGeometryMeshes.emplace(shape, meshes[0]);
			m_onLoadedMesh(meshes);
		}
		return m_primitiveGeometryMeshes[shape];
	}

	Image RenderResources::GetImage(const std::string& fileName)
	{
		auto image = m_images.find(fileName);
		if (image == m_images.end())
		{
			// ないなら読み込む
			m_images.emplace(fileName, Utility::Loader::loadImage(
				*m_device,
				m_device->GetTransferQueue(),
				m_device->GetVmaAllocator(),
				fileName,
				m_device->GetAllocationCallbacks()));
		}
		return { m_images[fileName].image,  m_images[fileName].view };
	}

	Image RenderResources::GetCubeImage(const std::vector<std::string>& fileNames)
	{
		auto image = m_cubeImages.find(fileNames[0]);
		if (image == m_cubeImages.end())
		{
			// ないなら読み込む
			m_cubeImages.emplace(fileNames[0], Utility::Loader::loadCubeImage(
				*m_device,
				m_device->GetTransferQueue(),
				m_device->GetVmaAllocator(),
				fileNames,
				m_device->GetAllocationCallbacks()));
		}
		return { m_cubeImages[fileNames[0]].image,  m_cubeImages[fileNames[0]].view };
	}

	void RenderResources::Resize(const uint32_t width, const uint32_t height)
	{
		{// attachment
			vmaDestroyImage(m_device->GetVmaAllocator(), m_mainRenderTarget.image, m_mainRenderTarget.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_visibilityBuffer.image, m_visibilityBuffer.allocation);

			vkDestroyImageView(*m_device, m_mainRenderTarget.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_visibilityBuffer.view, m_device->GetAllocationCallbacks());
		}
		Initialize(width, height);
	}

	VkSampler& RenderResources::CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo)
	{
		VkSampler sampler{};
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &sampler));
		m_samplers.push_back(sampler);
		return m_samplers.back();
	}

	bool RenderResources::SaveImage(const Image& image, const std::string& fileName, const glm::ivec2& size)
	{
		bool result = Utility::Loader::SaveImage(
			*m_device,
			image,
			m_device->GetGraphicsQueue(),
			m_device->GetVmaAllocator(),
			fileName,
			size,
			m_device->GetAllocationCallbacks());
		return result;
	}

	std::vector<Mesh> RenderResources::GetMeshFormID(const uint32_t meshID)
	{
		auto itm = std::find_if(m_meshes.begin(), m_meshes.end(),
			[&](std::pair<std::string, std::vector<Mesh>> pair)
			{
				for (Mesh& mesh : pair.second)
				{
					return mesh.meshInfo.meshID == meshID;
				}
			});
		if (itm != m_meshes.end())
		{
			return itm->second;
		}
		auto itp = std::find_if(m_primitiveGeometryMeshes.begin(), m_primitiveGeometryMeshes.end(),
			[&](std::pair<Shape::PrimitiveGeometryShape, Mesh> pair)
			{
				return pair.second.meshInfo.meshID == meshID;
			});
		if (itp != m_primitiveGeometryMeshes.end())
		{
			return { itp->second };
		}
		auto itt = std::find_if(m_terrains.begin(), m_terrains.end(),
			[&](std::pair<std::string, std::vector<Mesh>> pair)
			{
				for (Mesh& mesh : pair.second)
				{
					return mesh.meshInfo.meshID == meshID;
				}
			});
		if (itt != m_terrains.end())
		{
			return itt->second;
		}
		return {};
	}

	void RenderResources::createVertexIndexBuffer(std::vector<Mesh>& meshes)
	{
		for (Mesh& mesh : meshes)
		{
			{// vertex

				mesh.vertexBuffer = vmaTools::CreateBuffer(m_device->GetVmaAllocator(),
					sizeof(VertexData) * mesh.vertex.size(),
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
					VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);

				vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), mesh.vertexBuffer, mesh.vertex.data(), sizeof(VertexData) * mesh.vertex.size());
			}
			{// index
				mesh.indexBuffer = vmaTools::CreateBuffer(m_device->GetVmaAllocator(),
					sizeof(uint32_t) * mesh.index.size(),
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
					VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);

				vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), mesh.indexBuffer, mesh.index.data(), sizeof(uint32_t) * mesh.index.size());
			}
		}
	}
}