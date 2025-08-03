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
			MESH_DATA = 0,
			MESHLET_META_DATA,
			VERTEX_DATA,
			INDEX_DATA,
		};

		void FreeDescriptorSets(VkDescriptorSet& descriptorSet);
		void UpdateMainCameraData(const CameraData& cameraData);
		void UpdateMainDescriptorSet();

		void ResetMainDescriptorSetBuffer();

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

		void AddPrimitiveGeometry(std::vector<std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>> meshData);
		uint32_t AddCustomMesh(const std::string meshName, std::vector<Mesh> meshData);

		uint32_t AddCombinedImageSamplerInfo(const VkDescriptorImageInfo& imageInfo);
		uint32_t AddStorageImageInfo(const VkDescriptorImageInfo& imageInfo);

		VkDescriptorPool& GetDescriptorPool() { return m_descriptorPool; }
		VkDescriptorSet& GetBindlessMainDescriptorSet() { return m_mainDescriptorSet; }
		VkDescriptorSet& GetBindlessVertexDescriptorSet() { return m_vertexDescriptorSet; }
		VkDescriptorSetLayout& GetBindlessMainDescriptorSetLayout() { return m_mainDescriptorSetLayout; }
		VkDescriptorSetLayout& GetBindlessVertexDescriptorSetLayout() { return m_vertexDescriptorSetLayout; }

	private:
		struct MeshData {
			uint32_t meshID;				// MeshID
			uint32_t meshletMetaDataOffset;	// MeshletMetaDataの開始位置
			uint32_t meshletMetaDataCount;	// MeshletMetaDataの数
			uint32_t empty;					// 予約領域(将来の拡張用)
		};

		struct MeshletMetaData {
			uint32_t vertexCount;		// 頂点の数
			uint32_t primitiveCount;    // 三角形単位(三角形の数)
			uint32_t vertexAttributeBit;// 頂点属性のビットフラグ
			uint32_t unitSize;          // 一枚当たりのサイズ
			uint32_t vertexDataOffset;	// VertexDataの開始位置
			uint32_t indexDataOffset;	// IndexDataの開始位置
			uint32_t empty1;			// 予約領域(将来の拡張用)
			uint32_t empty2;			// 予約領域(将来の拡張用)
		};

	private:
		void createMainCameraBuffer();
		void createDescriptorPool();
		void createBindlessMainDescriptorSetLayout();
		void createBindlessVertexDescriptorSetLayout();
		void allocateMainDescriptorSet();
		void allocateVertexDescriptorSet();
		void updateVertexDescriptorSet();

		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_mainDescriptorSetLayout;
		VkDescriptorSetLayout m_vertexDescriptorSetLayout;
		VkDescriptorSet m_mainDescriptorSet;
		VkDescriptorSet m_vertexDescriptorSet;

		// Set = 0
		std::vector<uint32_t> m_storageBufferRawData;
		std::vector<MetaData> m_storageBufferMetaData;
		Buffer m_mainCameraDataBuffer;
		Buffer m_storageBufferRawDataBuffer;
		Buffer m_storageBufferMetaDataBuffer;

		// Set = 1
		std::vector<MeshData> m_meshDatas{};
		std::vector<MeshletMetaData> m_meshletMetaDatas{};
		std::vector<float> m_vertexDatas{};
		std::vector<uint32_t> m_indexDatas{};
		Buffer m_meshDataBuffer;
		Buffer m_meshletMetaDataBuffer;
		Buffer m_vertexDataBuffer;
		Buffer m_indexDataBuffer;

		std::vector<VkDescriptorImageInfo> m_combinedImageSamplersImageInfos;
		std::vector<VkDescriptorImageInfo> m_storageImageInfos;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDescriptors);
}