// Copyright (c) 2025 kong9812
#include "RenderDescriptors.h"
#include "RenderDevice.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "Appinfo.h"

namespace MyosotisFW::System::Render
{
	RenderDescriptors::RenderDescriptors(const RenderDevice_ptr& device) :
		m_device(device),
		m_descriptorPool(VK_NULL_HANDLE),
		m_mainDescriptorSetLayout(VK_NULL_HANDLE),
		m_mainDescriptorSet(VK_NULL_HANDLE),
		m_storageBufferRawData({}),
		m_storageBufferMetaData({}),
		m_mainCameraDataBuffer({}),
		m_storageBufferRawDataBuffer({}),
		m_storageBufferMetaDataBuffer({}),
		m_meshletMetaDataBuffer({}),
		m_vertexDataBuffer({}),
		m_indexDataBuffer({}) {
		createMainCameraBuffer();
		createDescriptorPool();
		createBindlessMainDescriptorSetLayout();
		createBindlessVertexDescriptorSetLayout();
		allocateMainDescriptorSet();
		allocateVertexDescriptorSet();
		m_storageBufferRawDataBuffer.buffer = VK_NULL_HANDLE;
		m_storageBufferMetaDataBuffer.buffer = VK_NULL_HANDLE;
		m_meshDataBuffer.buffer = VK_NULL_HANDLE;
		m_meshletMetaDataBuffer.buffer = VK_NULL_HANDLE;
		m_vertexDataBuffer.buffer = VK_NULL_HANDLE;
		m_indexDataBuffer.buffer = VK_NULL_HANDLE;
	}

	RenderDescriptors::~RenderDescriptors()
	{
		if (m_mainCameraDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_mainCameraDataBuffer.buffer, m_mainCameraDataBuffer.allocation);
			m_mainCameraDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_storageBufferRawDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_storageBufferRawDataBuffer.buffer, m_storageBufferRawDataBuffer.allocation);
			m_storageBufferRawDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_storageBufferMetaDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_storageBufferMetaDataBuffer.buffer, m_storageBufferMetaDataBuffer.allocation);
			m_storageBufferMetaDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_meshletMetaDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_meshletMetaDataBuffer.buffer, m_meshletMetaDataBuffer.allocation);
			m_meshletMetaDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_vertexDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexDataBuffer.buffer, m_vertexDataBuffer.allocation);
			m_vertexDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_indexDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexDataBuffer.buffer, m_indexDataBuffer.allocation);
			m_indexDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_meshDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_meshDataBuffer.buffer, m_meshDataBuffer.allocation);
			m_meshDataBuffer.buffer = VK_NULL_HANDLE;
		}
		vkDestroyDescriptorSetLayout(*m_device, m_vertexDescriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorSetLayout(*m_device, m_mainDescriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
	}

	void RenderDescriptors::createMainCameraBuffer()
	{
		// UBO作成
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			sizeof(CameraData),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_mainCameraDataBuffer.buffer,
			m_mainCameraDataBuffer.allocation,
			m_mainCameraDataBuffer.allocationInfo,
			m_mainCameraDataBuffer.descriptor);
	}

	void RenderDescriptors::createDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_device->GetMaxDescriptorSetUniformBuffers()),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_device->GetMaxDescriptorSetStorageBuffers()),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_device->GetMaxDescriptorSetSampledImages()),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_device->GetMaxDescriptorSetStorageImages()),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, m_device->GetMaxDescriptorSetInputAttachments()),
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, MyosotisFW::AppInfo::g_descriptorCount,
			VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));
	}

	void RenderDescriptors::createBindlessMainDescriptorSetLayout()
	{
		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: [UBO] MainCameraData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(MainDescriptorBindingIndex::MAIN_CAMERA_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: [SSBO] MetaData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(MainDescriptorBindingIndex::META_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: [SSBO] RawData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(MainDescriptorBindingIndex::STORAGE_BUFFER), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: CombinedImageSampler
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(MainDescriptorBindingIndex::COMBINED_IMAGE_SAMPLER), VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL, m_device->GetMaxDescriptorSetSampledImages()),
			// binding: StorageImage
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(MainDescriptorBindingIndex::STORAGE_IMAGE), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL, m_device->GetMaxDescriptorSetStorageImages()),
		};

		// 未使用許可 & バインド後更新 を有効化
		std::vector<VkDescriptorBindingFlags> descriptorBindingFlags = {
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{};
		descriptorSetLayoutBindingFlagsCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size());
		descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		descriptorSetLayoutCreateInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_mainDescriptorSetLayout));
	}

	void RenderDescriptors::createBindlessVertexDescriptorSetLayout()
	{
		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: [SSBO] MeshData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(VertexDescriptorBindingIndex::MESH_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: [SSBO] MeshletMetaData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(VertexDescriptorBindingIndex::MESHLET_META_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: [SSBO] VertexData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(VertexDescriptorBindingIndex::VERTEX_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
			// binding: [SSBO] IndexData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(static_cast<uint32_t>(VertexDescriptorBindingIndex::INDEX_DATA), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_ALL),
		};

		// 未使用許可 & バインド後更新 を有効化
		std::vector<VkDescriptorBindingFlags> descriptorBindingFlags = {
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{};
		descriptorSetLayoutBindingFlagsCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size());
		descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		descriptorSetLayoutCreateInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_vertexDescriptorSetLayout));
	}

	void RenderDescriptors::allocateMainDescriptorSet()
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_mainDescriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_mainDescriptorSet));
	}

	void RenderDescriptors::allocateVertexDescriptorSet()
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_vertexDescriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_vertexDescriptorSet));
	}

	void RenderDescriptors::updateVertexDescriptorSet()
	{
		// 全部クリアしてから
		if (m_meshDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_meshDataBuffer.buffer, m_meshDataBuffer.allocation);
			m_meshDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_meshletMetaDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_meshletMetaDataBuffer.buffer, m_meshletMetaDataBuffer.allocation);
			m_meshletMetaDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_vertexDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexDataBuffer.buffer, m_vertexDataBuffer.allocation);
			m_vertexDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_indexDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexDataBuffer.buffer, m_indexDataBuffer.allocation);
			m_indexDataBuffer.buffer = VK_NULL_HANDLE;
		}

		// writeDescriptorSet
		std::vector<VkWriteDescriptorSet> writeDescriptorSet{};

		// meshData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(MeshData) * static_cast<uint32_t>(m_meshDatas.size())),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_meshDataBuffer.buffer,
			m_meshDataBuffer.allocation,
			m_meshDataBuffer.allocationInfo,
			m_meshDataBuffer.descriptor);
		VkDescriptorBufferInfo meshDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_meshDataBuffer.buffer);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_vertexDescriptorSet,
			static_cast<uint32_t>(VertexDescriptorBindingIndex::MESH_DATA),
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &meshDataDescriptorBufferInfo));
		memcpy(m_meshDataBuffer.allocationInfo.pMappedData, m_meshDatas.data(),
			static_cast<uint32_t>(sizeof(MeshData) * static_cast<uint32_t>(m_meshDatas.size())));

		// meshletMetaData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(MeshletMetaData) * static_cast<uint32_t>(m_meshletMetaDatas.size())),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_vertexDataBuffer.buffer,
			m_vertexDataBuffer.allocation,
			m_vertexDataBuffer.allocationInfo,
			m_vertexDataBuffer.descriptor);
		VkDescriptorBufferInfo vertexMetaDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexDataBuffer.buffer);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_vertexDescriptorSet,
			static_cast<uint32_t>(VertexDescriptorBindingIndex::MESHLET_META_DATA),
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &vertexMetaDataDescriptorBufferInfo));
		memcpy(m_vertexDataBuffer.allocationInfo.pMappedData, m_meshletMetaDatas.data(),
			static_cast<uint32_t>(sizeof(MeshletMetaData) * static_cast<uint32_t>(m_meshletMetaDatas.size())));

		// vertexData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(float)) * static_cast<uint32_t>(m_vertexDatas.size()),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_meshletMetaDataBuffer.buffer,
			m_meshletMetaDataBuffer.allocation,
			m_meshletMetaDataBuffer.allocationInfo,
			m_meshletMetaDataBuffer.descriptor);
		VkDescriptorBufferInfo vertexDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_meshletMetaDataBuffer.buffer);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_vertexDescriptorSet,
			static_cast<uint32_t>(VertexDescriptorBindingIndex::VERTEX_DATA),
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &vertexDataDescriptorBufferInfo));
		memcpy(m_meshletMetaDataBuffer.allocationInfo.pMappedData, m_vertexDatas.data(),
			static_cast<uint32_t>(sizeof(float)) * static_cast<uint32_t>(m_vertexDatas.size()));

		// indexData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_indexDatas.size()),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_indexDataBuffer.buffer,
			m_indexDataBuffer.allocation,
			m_indexDataBuffer.allocationInfo,
			m_indexDataBuffer.descriptor);
		VkDescriptorBufferInfo indexDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexDataBuffer.buffer);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_vertexDescriptorSet,
			static_cast<uint32_t>(VertexDescriptorBindingIndex::INDEX_DATA),
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &indexDataDescriptorBufferInfo));
		memcpy(m_indexDataBuffer.allocationInfo.pMappedData, m_indexDatas.data(),
			static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_indexDatas.size()));

		// GPUへ!
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void RenderDescriptors::FreeDescriptorSets(VkDescriptorSet& descriptorSet)
	{
		if (descriptorSet != VK_NULL_HANDLE) {
			vkFreeDescriptorSets(*m_device, m_descriptorPool, 1, &descriptorSet);
			descriptorSet = VK_NULL_HANDLE;
		}
	}

	void RenderDescriptors::UpdateMainCameraData(const CameraData& cameraData)
	{
		memcpy(m_mainCameraDataBuffer.allocationInfo.pMappedData, &cameraData, sizeof(CameraData));
		VkWriteDescriptorSet writeDescriptorSet = Utility::Vulkan::CreateInfo::writeDescriptorSet(m_mainDescriptorSet,
			static_cast<uint32_t>(RenderDescriptors::MainDescriptorBindingIndex::MAIN_CAMERA_DATA),
			VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_mainCameraDataBuffer.descriptor);
		vkUpdateDescriptorSets(*m_device, 1, &writeDescriptorSet, 0, nullptr);
	}

	void RenderDescriptors::UpdateMainDescriptorSet()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSet{};
		VkDescriptorBufferInfo metaDataDescriptorBufferInfo{};
		VkDescriptorBufferInfo rawDataDescriptorBufferInfo{};

		if (m_storageBufferMetaData.size() > 0)
		{
			// SSBO作成
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				static_cast<uint32_t>(sizeof(MetaData) * static_cast<uint32_t>(m_storageBufferMetaData.size())),
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				m_storageBufferMetaDataBuffer.buffer,
				m_storageBufferMetaDataBuffer.allocation,
				m_storageBufferMetaDataBuffer.allocationInfo,
				m_storageBufferMetaDataBuffer.descriptor);
			metaDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_storageBufferMetaDataBuffer.buffer, 0);
			writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_mainDescriptorSet,
				static_cast<uint32_t>(RenderDescriptors::MainDescriptorBindingIndex::META_DATA),
				VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &metaDataDescriptorBufferInfo));

			memcpy(m_storageBufferMetaDataBuffer.allocationInfo.pMappedData, m_storageBufferMetaData.data(),
				static_cast<uint32_t>(sizeof(MetaData) * static_cast<uint32_t>(m_storageBufferMetaData.size())));
		}
		if (m_storageBufferRawData.size() > 0)
		{
			// SSBO作成
			vmaTools::ShaderBufferObjectAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				static_cast<uint32_t>(sizeof(uint32_t) * static_cast<uint32_t>(m_storageBufferRawData.size())),
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				m_storageBufferRawDataBuffer.buffer,
				m_storageBufferRawDataBuffer.allocation,
				m_storageBufferRawDataBuffer.allocationInfo,
				m_storageBufferRawDataBuffer.descriptor);
			rawDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_storageBufferRawDataBuffer.buffer, 0);
			writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_mainDescriptorSet,
				static_cast<uint32_t>(RenderDescriptors::MainDescriptorBindingIndex::STORAGE_BUFFER),
				VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &rawDataDescriptorBufferInfo));

			memcpy(m_storageBufferRawDataBuffer.allocationInfo.pMappedData, m_storageBufferRawData.data(),
				static_cast<uint32_t>(sizeof(uint32_t) * static_cast<uint32_t>(m_storageBufferRawData.size())));
		}
		if (m_combinedImageSamplersImageInfos.size() > 0)
		{
			writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_mainDescriptorSet,
				static_cast<uint32_t>(RenderDescriptors::MainDescriptorBindingIndex::COMBINED_IMAGE_SAMPLER),
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_combinedImageSamplersImageInfos.data(),
				static_cast<uint32_t>(m_combinedImageSamplersImageInfos.size())));
		}
		if (m_storageImageInfos.size() > 0)
		{
			writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_mainDescriptorSet,
				static_cast<uint32_t>(RenderDescriptors::MainDescriptorBindingIndex::STORAGE_IMAGE),
				VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_storageImageInfos.data(),
				static_cast<uint32_t>(m_storageImageInfos.size())));
		}
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void RenderDescriptors::ResetMainDescriptorSetBuffer()
	{
		m_storageBufferMetaData.clear();
		m_storageBufferRawData.clear();
		m_combinedImageSamplersImageInfos.clear();
		m_storageImageInfos.clear();
		if (m_storageBufferRawDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_storageBufferRawDataBuffer.buffer, m_storageBufferRawDataBuffer.allocation);
			m_storageBufferRawDataBuffer.buffer = VK_NULL_HANDLE;
		}
		if (m_storageBufferMetaDataBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_storageBufferMetaDataBuffer.buffer, m_storageBufferMetaDataBuffer.allocation);
			m_storageBufferMetaDataBuffer.buffer = VK_NULL_HANDLE;
		}
	}

	void RenderDescriptors::AddPrimitiveGeometry(std::vector<std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>> meshDatas)
	{
		for (const auto& [shape, meshes] : meshDatas)
		{
			// MeshData
			MeshData meshData{};
			meshData.meshID = static_cast<uint32_t>(m_meshDatas.size()); // 最後に追加されたMeshのID
			meshData.meshletMetaDataOffset = static_cast<uint32_t>(m_meshletMetaDatas.size()); // MeshMetaDataの開始位置

			for (const Mesh& mesh : meshes)
			{
				// MeshletMetaData
				MeshletMetaData meshletMetaData{};
				meshletMetaData.vertexCount = mesh.vertex.size() / 13; // (x,y,z,w,uv1X....)
				meshletMetaData.primitiveCount = mesh.index.size() / 3; // 三角形
				meshletMetaData.vertexAttributeBit = Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 | Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL | Utility::Vulkan::CreateInfo::VertexAttributeBit::UV | Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;
				meshletMetaData.unitSize = 13;
				meshletMetaData.vertexDataOffset = m_vertexDatas.size();
				meshletMetaData.indexDataOffset = m_indexDatas.size();	// 今後は計算しないといけない(追加されたときにindexを取るように)
				m_meshletMetaDatas.push_back(meshletMetaData);

				// VertexData
				m_vertexDatas.insert(m_vertexDatas.end(), mesh.vertex.begin(), mesh.vertex.end());

				// IndexData
				m_indexDatas.insert(m_indexDatas.end(), mesh.index.begin(), mesh.index.end());
			}

			meshData.meshletMetaDataCount = static_cast<uint32_t>(meshes.size()); // MeshMetaDataの数
			m_meshDatas.push_back(meshData);
		}
		updateVertexDescriptorSet();
	}

	uint32_t RenderDescriptors::AddCustomMesh(const std::string meshName, std::vector<Mesh> meshDatas)
	{
		// MeshData
		MeshData meshData{};
		meshData.meshID = static_cast<uint32_t>(m_meshDatas.size()); // 最後に追加されたMeshのID
		meshData.meshletMetaDataOffset = static_cast<uint32_t>(m_meshletMetaDatas.size()); // MeshMetaDataの開始位置

		for (const Mesh& mesh : meshDatas)
		{
			// MeshletMetaData
			MeshletMetaData meshletMetaData{};
			meshletMetaData.vertexCount = mesh.vertex.size() / 13; // (x,y,z,w,uv1X....)
			meshletMetaData.primitiveCount = mesh.index.size() / 3; // 三角形
			meshletMetaData.vertexAttributeBit = Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 | Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL | Utility::Vulkan::CreateInfo::VertexAttributeBit::UV | Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;
			meshletMetaData.unitSize = 13;
			meshletMetaData.vertexDataOffset = m_vertexDatas.size();
			m_meshletMetaDatas.push_back(meshletMetaData);

			// VertexData
			m_vertexDatas.insert(m_vertexDatas.end(), mesh.vertex.begin(), mesh.vertex.end());

			// IndexData
			m_indexDatas.insert(m_indexDatas.end(), mesh.index.begin(), mesh.index.end());
		}
		meshData.meshletMetaDataCount = static_cast<uint32_t>(meshDatas.size()); // MeshMetaDataの数
		m_meshDatas.push_back(meshData);

		return 0;
	}

	uint32_t RenderDescriptors::AddCombinedImageSamplerInfo(const VkDescriptorImageInfo& imageInfo)
	{
		uint32_t index = m_combinedImageSamplersImageInfos.size();

		// 同じimageViewがあるかどうかを確認重複画像を再度入れないように
		auto it = std::find_if(m_combinedImageSamplersImageInfos.begin(), m_combinedImageSamplersImageInfos.end(), [=](VkDescriptorImageInfo v)
			{
				return v.imageView == imageInfo.imageView;
			});
		if (it != m_combinedImageSamplersImageInfos.end())
		{
			return static_cast<uint32_t>(std::distance(m_combinedImageSamplersImageInfos.begin(), it));
		}
		else
		{
			m_combinedImageSamplersImageInfos.push_back(imageInfo);
		}
		return index;
	}

	uint32_t RenderDescriptors::AddStorageImageInfo(const VkDescriptorImageInfo& imageInfo)
	{
		uint32_t index = m_storageImageInfos.size();
		// 同じimageViewがあるかどうかを確認重複画像を再度入れないように
		auto it = std::find_if(m_storageImageInfos.begin(), m_storageImageInfos.end(), [=](VkDescriptorImageInfo v)
			{
				return v.imageView == imageInfo.imageView;
			});
		if (it != m_storageImageInfos.end())
		{
			return static_cast<uint32_t>(std::distance(m_storageImageInfos.begin(), it));
		}
		else
		{
			m_storageImageInfos.push_back(imageInfo);
		}
		return index;
	}
}