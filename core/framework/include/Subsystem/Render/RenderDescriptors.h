// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Structs.h"
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class RenderDescriptors
	{
	public:
		RenderDescriptors(const RenderDevice_ptr& device);
		~RenderDescriptors();

		enum class DescriptorBindingIndex : uint8_t
		{
			META_DATA = 0,
			STORAGE_BUFFER,
			COMBINED_IMAGE_SAMPLER,
			STORAGE_IMAGE,
		};

		void FreeDescriptorSets(VkDescriptorSet& descriptorSet);
		void UpdateDescriptorSet();

		void ResetInfos();

		template<typename T>
		uint32_t AddStorageBuffer(const T& object)
		{
			uint32_t index = m_storageBufferMetaData.size();
			uint32_t dataOffset = m_storageBufferRawData.size();

			const uint32_t* raw = reinterpret_cast<const uint32_t*>(&object);
			size_t wordCount = sizeof(T) / sizeof(uint32_t);
			m_storageBufferRawData.insert(m_storageBufferRawData.end(), raw, raw + wordCount);

			MetaData metaData{};
			metaData.typeID = 0;
			metaData.dataOffset = dataOffset;
			m_storageBufferMetaData.push_back(metaData);
			return index;
		}

		uint32_t AddCombinedImageSamplerInfo(const VkDescriptorImageInfo& imageInfo);
		uint32_t AddStorageImageInfo(const VkDescriptorImageInfo& imageInfo);

		VkDescriptorSet& GetBindlessDescriptorSet() { return m_descriptorSet; }
		VkDescriptorPool& GetDescriptorPool() { return m_descriptorPool; }
		VkDescriptorSetLayout& GetBindlessDescriptorSetLayout() { return m_descriptorSetLayout; }

	private:
		void createDescriptorPool();
		void createBindlessDescriptorSetLayout();
		void allocateDescriptorSet();

		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;

		std::vector<uint32_t> m_storageBufferRawData;
		std::vector<MetaData> m_storageBufferMetaData;
		Buffer m_storageBufferRawDataBuffer;
		Buffer m_storageBufferMetaDataBuffer;

		std::vector<VkDescriptorImageInfo> m_combinedImageSamplersImageInfos;
		std::vector<VkDescriptorImageInfo> m_storageImageInfos;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDescriptors);
}