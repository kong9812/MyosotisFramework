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
		// attachment
		destroyAllAttachment();

		{// Dummy images
			vkDestroySampler(*m_device, m_dummyImages.sampled2D.sampler, m_device->GetAllocationCallbacks());
			vmaDestroyImage(m_device->GetVmaAllocator(), m_dummyImages.sampled2D.image, m_dummyImages.sampled2D.allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_dummyImages.storage2D.image, m_dummyImages.storage2D.allocation);
			vkDestroyImageView(*m_device, m_dummyImages.sampled2D.view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_dummyImages.storage2D.view, m_device->GetAllocationCallbacks());
		}

		for (std::pair<std::string, VkShaderModule> shaderModule : m_shaderModules)
		{
			vkDestroyShaderModule(*m_device, shaderModule.second, m_device->GetAllocationCallbacks());
		}
		m_shaderModules.clear();

		for (std::pair<std::string, MeshesData> meshList : m_meshes)
		{
			for (Mesh_ptr& mesh : meshList.second.mesh)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), mesh->indexBuffer.buffer, mesh->indexBuffer.allocation);
			}
		}
		m_meshes.clear();

		for (std::pair<Shape::PrimitiveGeometryShape, MeshData> meshPair : m_primitiveGeometryMeshes)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), meshPair.second.mesh->vertexBuffer.buffer, meshPair.second.mesh->vertexBuffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), meshPair.second.mesh->indexBuffer.buffer, meshPair.second.mesh->indexBuffer.allocation);
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

	void RenderResources::Initialize(const glm::ivec2& screenSize)
	{
		// createAttachment
		createAttachment(screenSize);

		// create dummy images
		createDummyImages();

		// Create Default Material
		createDefaultMaterial();
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

	MeshesHandle RenderResources::GetMesh(const FilePath& filePath)
	{
		// 絶対パス
		std::filesystem::path path = filePath.GetFilesystemPath();
		if (!path.is_absolute())
		{
			path = std::filesystem::absolute(std::filesystem::path(MyosotisFW::AppInfo::g_terrainFolder) / path);
		}

		auto vertexData = m_meshes.find(path.string());
		if (vertexData == m_meshes.end())
		{
			// 拡張子判定
			std::string extension = path.extension().string();
			std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> rawMeshes{};
			if ((extension == ".fbx") || (extension == ".FBX"))
			{
				rawMeshes = Utility::Loader::loadFbx(path);
			}
			else if ((extension == ".gltf") || (extension == ".GLTF"))
			{
				rawMeshes = Utility::Loader::loadGltf(path);
			}
			else if ((extension == ".mfmodel") || (extension == ".MFMODEL"))
			{
				rawMeshes = Utility::Loader::loadMFModel(path);
			}
			else
			{
				ASSERT(false, "Unsupported mesh file format: " + path.filename().string());
			}

			if (rawMeshes.size() <= 0) return {};	// 何もロードできない

			MeshesData meshes{};
			MaterialData materials{};
			uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size();
			uint32_t materialID = m_primitiveGeometryMaterials.size() + m_materials.size();
			for (auto& [mesh, material] : rawMeshes)
			{
				// Mesh
				Mesh_ptr meshPtr = std::make_shared<Mesh>(std::move(mesh));
				meshPtr->meshInfo.meshID = meshID;
				meshes.mesh.push_back(meshPtr);
				meshes.meshHandle.push_back(meshPtr);
				meshID++;

				// Material
				if (!material.isDefault())
				{
					BasicMaterial_ptr matPtr = std::make_shared<BasicMaterial>(std::move(material));
					if (!matPtr->baseColorTexturePath.empty())
					{
						std::filesystem::path modelFolder(AppInfo::g_modelFolder);
						std::filesystem::path imagePath = modelFolder.append(path.parent_path().string());
						imagePath = imagePath.append(matPtr->baseColorTexturePath);
						Image image = GetImage(imagePath.string());
						VkSampler& sampler = CreateSampler(Utility::Vulkan::CreateInfo::samplerCreateInfo());
						VkDescriptorImageInfo imageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(sampler, image.view, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
						matPtr->basicMaterialInfo.baseColorTexture = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, imageInfo);
					}
					materials.material.push_back(matPtr);
					materials.materialHandle.push_back(matPtr);

					meshPtr->meshInfo.materialID = materialID;
					materialID++;
				}
				else
				{
					meshPtr->meshInfo.materialID = 0;
				}
			}
			createVertexIndexBuffer(meshes.mesh);
			m_onLoadedMesh(meshes.meshHandle);

			m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(path.string(), meshes.meshHandle);
			m_meshes.emplace(path.string(), std::move(meshes));

			if (materials.material.size() > 0)
			{
				m_renderDescriptors->GetMaterialDescriptorSet()->AddBasicMaterial(materials.materialHandle);
				m_materials.emplace(path.string(), std::move(materials));
			}
		}
		return m_meshes[path.string()].meshHandle;
	}

	MeshesHandle RenderResources::GetTerrainMesh(const FilePath& fileName)
	{
		// 絶対パス
		std::filesystem::path path = fileName.GetFilesystemPath();
		if (!path.is_absolute())
		{
			path = std::filesystem::absolute(std::filesystem::path(MyosotisFW::AppInfo::g_terrainFolder) / path);
		}

		if (!std::filesystem::exists(path))
		{
			return {};
		}

		auto vertexData = m_meshes.find(path.string());
		if (vertexData == m_meshes.end())
		{
			// 拡張子判定
			std::string extension = path.extension().string();
			std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> rawMeshes{};
			if ((extension == ".png") || (extension == ".PNG"))
			{
				rawMeshes = Utility::Loader::loadTerrainMesh(path);
			}
			else
			{
				ASSERT(false, "Unsupported terrain format: " + path.string());
			}

			if (rawMeshes.size() <= 0) return {};	// 何もロードできない

			MeshesData meshes{};
			MaterialData materials{};
			uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size();
			uint32_t materialID = m_primitiveGeometryMaterials.size() + m_materials.size();
			for (auto& [mesh, material] : rawMeshes)
			{
				// Mesh
				Mesh_ptr meshPtr = std::make_shared<Mesh>(std::move(mesh));
				meshPtr->meshInfo.meshID = meshID;
				meshes.mesh.push_back(meshPtr);
				meshes.meshHandle.push_back(meshPtr);
				meshID++;

				// Material
				if (!material.isDefault())
				{
					BasicMaterial_ptr matPtr = std::make_shared<BasicMaterial>(std::move(material));
					materials.material.push_back(matPtr);
					materials.materialHandle.push_back(matPtr);
					meshPtr->meshInfo.materialID = materialID;
					materialID++;
				}
				else
				{
					meshPtr->meshInfo.materialID = 0;
				}
			}
			createVertexIndexBuffer(meshes.mesh);
			m_onLoadedMesh(meshes.meshHandle);

			m_renderDescriptors->GetMeshInfoDescriptorSet()->AddCustomGeometry(path.string(), meshes.meshHandle);
			m_meshes.emplace(path.string(), std::move(meshes));

			if (materials.material.size() > 0)
			{
				m_renderDescriptors->GetMaterialDescriptorSet()->AddBasicMaterial(materials.materialHandle);
				m_materials.emplace(path.string(), std::move(materials));
			}
		}
		return m_meshes[path.string()].meshHandle;
	}

	MeshHandle RenderResources::GetPrimitiveGeometryMesh(const Shape::PrimitiveGeometryShape shape)
	{
		auto vertexData = m_primitiveGeometryMeshes.find(shape);
		if (vertexData == m_primitiveGeometryMeshes.end())
		{
			// [仮] Material
			BasicMaterial material{};
			material.basicMaterialInfo.bitFlags = 0;
			material.basicMaterialInfo.baseColor = glm::vec4(1.0f);

			// Mesh
			Mesh rawMesh = Shape::createShape(shape);
			MeshData mesh{};
			mesh.mesh = std::make_shared<Mesh>(std::move(rawMesh));
			mesh.meshHandle = mesh.mesh;
			uint32_t meshID = m_primitiveGeometryMeshes.size() + m_meshes.size();
			mesh.mesh->meshInfo.meshID = meshID;
			Meshes meshes{ mesh.mesh };
			MeshesHandle meshesHandle{ mesh.mesh };

			// Material
			MaterialData materials{};
			if (!material.isDefault())
			{
				BasicMaterial_ptr materialPtr = std::make_shared<BasicMaterial>(std::move(material));
				materials.material = { materialPtr };
				materials.materialHandle = { materialPtr };
				uint32_t materialID = m_primitiveGeometryMaterials.size() + m_materials.size();
				mesh.mesh->meshInfo.materialID = materialID;
			}

			createVertexIndexBuffer(meshes);
			m_onLoadedMesh(meshesHandle);

			m_renderDescriptors->GetMeshInfoDescriptorSet()->AddPrimitiveGeometry(shape, mesh.meshHandle);
			m_primitiveGeometryMeshes.emplace(shape, std::move(mesh));

			if (materials.material.size() > 0)
			{
				m_renderDescriptors->GetMaterialDescriptorSet()->AddBasicMaterial(materials.materialHandle);
				m_primitiveGeometryMaterials.emplace(shape, std::move(materials));
			}
		}
		return m_primitiveGeometryMeshes[shape].meshHandle;
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

	void RenderResources::Resize(const glm::ivec2& screenSize)
	{
		// attachment
		destroyAllAttachment();

		// createAttachment
		createAttachment(screenSize);
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

	MeshHandle RenderResources::GetMeshFormID(const uint32_t meshID)
	{
		for (const auto& [key, value] : m_meshes)
		{
			for (size_t i = 0; i < value.mesh.size(); i++)
			{
				const Mesh_ptr& mesh = value.mesh[i];
				if (mesh->meshInfo.meshID == meshID)
				{
					return value.meshHandle[i];
				}
			}
		}
		for (const auto& [key, mesh] : m_primitiveGeometryMeshes)
		{
			if (mesh.mesh->meshInfo.meshID == meshID)
			{
				return mesh.meshHandle;
			}
		}
		return {};
	}

	void RenderResources::destroyAllAttachment()
	{
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			vkDestroySampler(*m_device, m_hiZDepthMap[i].sampler, m_device->GetAllocationCallbacks());
			vkDestroySampler(*m_device, m_depthBuffer[i].sampler, m_device->GetAllocationCallbacks());

			vmaDestroyImage(m_device->GetVmaAllocator(), m_mainRenderTarget[i].image, m_mainRenderTarget[i].allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_visibilityBuffer[i].image, m_visibilityBuffer[i].allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_hiZDepthMap[i].image, m_hiZDepthMap[i].allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_depthBuffer[i].image, m_depthBuffer[i].allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_lightmap[i].image, m_lightmap[i].allocation);
			vmaDestroyImage(m_device->GetVmaAllocator(), m_rayTracingRenderTarget[i].image, m_rayTracingRenderTarget[i].allocation);

			vkDestroyImageView(*m_device, m_mainRenderTarget[i].view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_visibilityBuffer[i].view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_hiZDepthMap[i].view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_lightmap[i].view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_rayTracingRenderTarget[i].view, m_device->GetAllocationCallbacks());
			for (VkImageView& view : m_hiZDepthMap[i].mipView)
				vkDestroyImageView(*m_device, view, m_device->GetAllocationCallbacks());
			vkDestroyImageView(*m_device, m_depthBuffer[i].view, m_device->GetAllocationCallbacks());
		}
	}

	void RenderResources::createVertexIndexBuffer(Meshes& meshes)
	{
		for (Mesh_ptr& mesh : meshes)
		{
			{// vertex

				mesh->vertexBuffer = vmaTools::CreateBuffer(m_device->GetVmaAllocator(),
					sizeof(VertexData) * mesh->vertex.size(),
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
					VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);

				vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), mesh->vertexBuffer, mesh->vertex.data(), sizeof(VertexData) * mesh->vertex.size());
			}
			{// index
				mesh->indexBuffer = vmaTools::CreateBuffer(m_device->GetVmaAllocator(),
					sizeof(uint32_t) * mesh->index.size(),
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
					VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);

				vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), mesh->indexBuffer, mesh->index.data(), sizeof(uint32_t) * mesh->index.size());
			}
		}
	}

	void RenderResources::createAttachment(const glm::ivec2& screenSize)
	{
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			{// main render target
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, screenSize.x, screenSize.y);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_mainRenderTarget[i].image, &m_mainRenderTarget[i].allocation, &m_mainRenderTarget[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_mainRenderTarget[i].image, AppInfo::g_surfaceFormat.format);
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_mainRenderTarget[i].view));
			}
			{// Visibility Buffer
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_visibilityBufferFormat, screenSize.x, screenSize.y);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_visibilityBuffer[i].image, &m_visibilityBuffer[i].allocation, &m_visibilityBuffer[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_visibilityBuffer[i].image, AppInfo::g_visibilityBufferFormat);
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_visibilityBuffer[i].view));
			}

			{// Hi-Z DepthMap
				uint32_t hiZMipLevels = floor(log2(std::max(screenSize.x, screenSize.y)));
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForHiZDepthStencil(AppInfo::g_hiZDepthFormat, screenSize.x, screenSize.y, hiZMipLevels);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT |
					VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_hiZDepthMap[i].image, &m_hiZDepthMap[i].allocation, &m_hiZDepthMap[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_hiZDepthMap[i].image, AppInfo::g_hiZDepthFormat);
				imageViewCreateInfo.subresourceRange.levelCount = hiZMipLevels;
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap[i].view));
				VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
				samplerCreateInfo.compareEnable = VK_FALSE;
				samplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
				VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap[i].sampler));
				m_hiZDepthMap[i].mipView.resize(hiZMipLevels);
				for (uint8_t j = 0; j < hiZMipLevels; j++)
				{
					VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_hiZDepthMap[i].image, AppInfo::g_hiZDepthFormat);
					imageViewCreateInfo.subresourceRange.baseMipLevel = j;
					VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_hiZDepthMap[i].mipView[j]));
				}
			}

			{// DepthBuffer
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForDepthStencil(AppInfo::g_depthBufferFormat, screenSize.x, screenSize.y);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_depthBuffer[i].image, &m_depthBuffer[i].allocation, &m_depthBuffer[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForDepth(m_depthBuffer[i].image, AppInfo::g_depthBufferFormat);
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_depthBuffer[i].view));
				VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
				VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_depthBuffer[i].sampler));
			}

			{// Lightmap
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_lightmapFormat, AppInfo::g_lightmapSize, AppInfo::g_lightmapSize);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_lightmap[i].image, &m_lightmap[i].allocation, &m_lightmap[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_lightmap[i].image, AppInfo::g_lightmapFormat);
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_lightmap[i].view));
			}

			{// ray tracing render target
				VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_rayTracingRenderTargetFormat, screenSize.x, screenSize.y);
				imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_rayTracingRenderTarget[i].image, &m_rayTracingRenderTarget[i].allocation, &m_rayTracingRenderTarget[i].allocationInfo));
				VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_rayTracingRenderTarget[i].image, AppInfo::g_rayTracingRenderTargetFormat);
				VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_rayTracingRenderTarget[i].view));
			}
		}
	}

	void RenderResources::createDefaultMaterial()
	{
		BasicMaterial defaultMaterial{};
		defaultMaterial.basicMaterialInfo.baseColor = glm::vec4(1.0f);

		MaterialData material{};
		BasicMaterial_ptr matPtr = std::make_shared<BasicMaterial>(std::move(defaultMaterial));
		material.material.push_back(matPtr);
		material.materialHandle.push_back(matPtr);
		m_renderDescriptors->GetMaterialDescriptorSet()->AddBasicMaterial(material.materialHandle);
		m_materials.emplace("default", std::move(material));
	}

	void RenderResources::createDummyImages()
	{
		{// Sampled2D
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(VkFormat::VK_FORMAT_R8G8B8A8_UNORM, 1, 1);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_dummyImages.sampled2D.image, &m_dummyImages.sampled2D.allocation, &m_dummyImages.sampled2D.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_dummyImages.sampled2D.image, VkFormat::VK_FORMAT_R8G8B8A8_UNORM);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_dummyImages.sampled2D.view));
			VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
			VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_dummyImages.sampled2D.sampler));

			VkDescriptorImageInfo imageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(m_dummyImages.sampled2D.sampler, m_dummyImages.sampled2D.view, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			m_renderDescriptors->GetTextureDescriptorSet()->SetDummySampled2D(imageInfo);
		}
		{// Storage2D
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(VkFormat::VK_FORMAT_R8G8B8A8_UNORM, 1, 1);
			imageCreateInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_dummyImages.storage2D.image, &m_dummyImages.storage2D.allocation, &m_dummyImages.storage2D.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_dummyImages.storage2D.image, VkFormat::VK_FORMAT_R8G8B8A8_UNORM);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_dummyImages.storage2D.view));

			VkDescriptorImageInfo imageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, m_dummyImages.storage2D.view, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
			m_renderDescriptors->GetTextureDescriptorSet()->SetDummyStorage2D(imageInfo);
		}

		// DataSet
		VkImageMemoryBarrier barriers[2]{};
		{// Sampled2D
			barriers[0].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
			barriers[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[0].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[0].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = m_dummyImages.sampled2D.image;
			barriers[0].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		{// Storage2D
			barriers[1].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[1].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
			barriers[1].dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[1].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[1].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = m_dummyImages.storage2D.image;
			barriers[1].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);

		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 2, barriers);

		VkClearColorValue clearColor{};
		clearColor.float32[0] = 0.0f;
		clearColor.float32[1] = 0.0f;
		clearColor.float32[2] = 0.0f;
		clearColor.float32[3] = 1.0f;
		vkCmdClearColorImage(
			commandBuffer,
			m_dummyImages.sampled2D.image,
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor,
			1,
			&barriers[0].subresourceRange
		);
		vkCmdClearColorImage(
			commandBuffer,
			m_dummyImages.storage2D.image,
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor,
			1,
			&barriers[1].subresourceRange
		);

		// Sampled2D
		barriers[0].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barriers[0].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barriers[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barriers[0]);

		// Storage2D
		barriers[1].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barriers[1].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
		barriers[1].srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[1].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT | VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barriers[1]);

		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		VkFence fence = VK_NULL_HANDLE;
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));
		m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
		VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));
		m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
		vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
	}
}