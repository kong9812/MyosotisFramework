// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Structs.h"
#include "ClassPointer.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class RenderDescriptors
	{
	public:
		RenderDescriptors(const RenderDevice_ptr& device);
		~RenderDescriptors();

		enum class MainDescriptorBindingIndex : uint32_t
		{
			MAIN_CAMERA_DATA = 0,
			META_DATA,
			STORAGE_BUFFER,
			COMBINED_IMAGE_SAMPLER,
			STORAGE_IMAGE,
		};
		enum class VertexDescriptorBindingIndex : uint32_t
		{
			VERTEX_META_DATA = 0,
			VERTEX_DATA,
			INDEX_META_DATA,
			INDEX_DATA,
		};

		void FreeDescriptorSets(VkDescriptorSet& descriptorSet);
		void UpdateMainCameraData(const CameraData& cameraData);
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

		void AddPrimitiveGeometryModel(std::vector<std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>> meshData);
		uint32_t AddCombinedImageSamplerInfo(const VkDescriptorImageInfo& imageInfo);
		uint32_t AddStorageImageInfo(const VkDescriptorImageInfo& imageInfo);

		VkDescriptorPool& GetDescriptorPool() { return m_descriptorPool; }
		VkDescriptorSet& GetBindlessMainDescriptorSet() { return m_mainDescriptorSet; }
		VkDescriptorSet& GetBindlessVertexDescriptorSet() { return m_vertexDescriptorSet; }
		VkDescriptorSetLayout& GetBindlessMainDescriptorSetLayout() { return m_mainDescriptorSetLayout; }
		VkDescriptorSetLayout& GetBindlessVertexDescriptorSetLayout() { return m_vertexDescriptorSetLayout; }

	private:
		struct VertexMetaData {
			uint32_t vertexCount;
			uint32_t primitiveCount;    // 三角形単位(三角形の数)
			uint32_t vertexAttributeBit;
			uint32_t unitSize;          // 一枚当たりのサイズ
			uint32_t offset;
		};

		struct IndexMetaData {
			uint32_t offset;            // IndexDataの開始位置
		};

	private:
		void createMainCameraBuffer();
		void createDescriptorPool();
		void createBindlessMainDescriptorSetLayout();
		void createBindlessVertexDescriptorSetLayout();
		void allocateMainDescriptorSet();
		void allocateVertexDescriptorSet();

		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_mainDescriptorSetLayout;
		VkDescriptorSetLayout m_vertexDescriptorSetLayout;
		VkDescriptorSet m_mainDescriptorSet;
		VkDescriptorSet m_vertexDescriptorSet;

		std::vector<uint32_t> m_storageBufferRawData;
		std::vector<MetaData> m_storageBufferMetaData;
		Buffer m_mainCameraDataBuffer;
		Buffer m_storageBufferRawDataBuffer;
		Buffer m_storageBufferMetaDataBuffer;

		Buffer m_vertexMetaDataBuffer;
		Buffer m_vertexDataBuffer;
		Buffer m_indexMetaDataBuffer;
		Buffer m_indexDataBuffer;

		std::vector<VkDescriptorImageInfo> m_combinedImageSamplersImageInfos;
		std::vector<VkDescriptorImageInfo> m_storageImageInfos;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDescriptors);
}