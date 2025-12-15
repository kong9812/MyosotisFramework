// Copyright (c) 2025 kong9812
#include "MeshInfoDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	MeshInfoDescriptorSet::MeshInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_meshInfo(),
		m_meshletInfo(),
		m_vertexData(),
		m_uniqueIndexData(),
		m_primitivesData(),
		m_primitiveMeshIDTable(),
		m_customMeshIDTable()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
		{
			Descriptor descriptor{};
			descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			descriptor.descriptorCount = 1;
			descriptor.rebuild = true;
			descriptor.update = true;
			m_descriptors.push_back(descriptor);
		}
		createDescriptorSet();
	}

	void MeshInfoDescriptorSet::Update()
	{
		// [Descriptor作成] MeshInfo
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(MeshInfo)) * static_cast<uint32_t>(m_meshInfo.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].rebuild = false;
		}
		// [Descriptor作成] MeshletInfo
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(MeshletInfo)) * static_cast<uint32_t>(m_meshletInfo.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].rebuild = false;
		}
		// [Descriptor作成] VertexData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(VertexData)) * static_cast<uint32_t>(m_vertexData.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::VertexData), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].rebuild = false;
		}
		// [Descriptor作成] UniqueIndexData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_uniqueIndexData.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].rebuild = false;
		}
		// [Descriptor作成] PrimitivesData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_primitivesData.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].rebuild = false;
		}

		// [更新] MeshInfo
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].update)
		{
			updateMeshInfo();
		}
		// [更新] MeshletInfo
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].update)
		{
			updateMeshletInfo();
		}
		// [更新] VertexData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].update)
		{
			updateVertexData();
		}
		// [更新] UniqueIndexData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].update)
		{
			updateUniqueIndexData();
		}
		// [更新] PrimitivesData
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].update)
		{
			updatePrimitivesData();
		}
	}

	uint32_t MeshInfoDescriptorSet::AddPrimitiveGeometry(const Shape::PrimitiveGeometryShape shape, const Mesh& mesh)
	{
		// データの再利用
		auto it = m_primitiveMeshIDTable.find(shape);
		if (it != m_primitiveMeshIDTable.end())
		{
			return it->second;
		}

		// MeshInfo
		MeshInfo meshInfo{};
		meshInfo.meshID = static_cast<uint32_t>(m_meshInfo.size()); // 最後に追加されたMeshのID
		meshInfo.meshletInfoOffset = static_cast<uint32_t>(m_meshletInfo.size()); // MeshMetaDataの開始位置
		meshInfo.AABBMin = mesh.meshInfo.AABBMin;
		meshInfo.AABBMax = mesh.meshInfo.AABBMax;
		meshInfo.vertexAttributeBit =
			Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC3 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV0 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV1 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;
		meshInfo.unitSize = sizeof(VertexData) / sizeof(float);
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			// MeshletMetaData
			MeshletInfo meshletInfo{};
			meshletInfo.meshID = meshInfo.meshID;
			meshletInfo.AABBMin = meshlet.meshletInfo.AABBMin;
			meshletInfo.AABBMax = meshlet.meshletInfo.AABBMax;
			meshletInfo.vertexCount = meshlet.uniqueIndex.size(); // (x,y,z,w,uv1X....)
			meshletInfo.primitiveCount = meshlet.primitives.size() / 3; // 三角形
			meshletInfo.vertexDataOffset = m_vertexData.size() * (sizeof(VertexData) / sizeof(float));
			meshletInfo.uniqueIndexOffset = m_uniqueIndexData.size();
			meshletInfo.primitivesOffset = m_primitivesData.size();
			m_meshletInfo.push_back(meshletInfo);

			// uniqueIndex
			m_uniqueIndexData.insert(m_uniqueIndexData.end(), meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end());

			// primitive
			m_primitivesData.insert(m_primitivesData.end(), meshlet.primitives.begin(), meshlet.primitives.end());
		}

		// VertexData
		m_vertexData.insert(m_vertexData.end(), mesh.vertex.begin(), mesh.vertex.end());
		meshInfo.meshletCount = static_cast<uint32_t>(m_meshletInfo.size()) - meshInfo.meshletInfoOffset; // MeshMetaDataの個数
		m_meshInfo.push_back(meshInfo);

		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].rebuild = true;

		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].update = true;

		m_primitiveMeshIDTable.emplace(shape, meshInfo.meshID);
		return meshInfo.meshID;
	}

	std::vector<uint32_t> MeshInfoDescriptorSet::AddCustomGeometry(const std::string name, const std::vector<Mesh>& meshes)
	{
		// データの再利用
		auto it = m_customMeshIDTable.find(name);
		if (it != m_customMeshIDTable.end())
		{
			return it->second;
		}

		std::vector<uint32_t> meshIDs{};
		for (const Mesh& mesh : meshes)
		{
			// MeshInfo
			MeshInfo meshInfo{};
			meshInfo.meshID = static_cast<uint32_t>(m_meshInfo.size()); // 最後に追加されたMeshのID
			meshInfo.meshletInfoOffset = static_cast<uint32_t>(m_meshletInfo.size()); // MeshMetaDataの開始位置
			meshInfo.AABBMin = mesh.meshInfo.AABBMin;
			meshInfo.AABBMax = mesh.meshInfo.AABBMax;
			meshInfo.vertexAttributeBit =
				Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC3 |
				Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL |
				Utility::Vulkan::CreateInfo::VertexAttributeBit::UV0 |
				Utility::Vulkan::CreateInfo::VertexAttributeBit::UV1 |
				Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;
			meshInfo.unitSize = sizeof(VertexData) / sizeof(float);
			for (const Meshlet& meshlet : mesh.meshlet)
			{
				// MeshletMetaData
				MeshletInfo meshletInfo{};
				meshletInfo.meshID = meshInfo.meshID;
				meshletInfo.AABBMin = meshlet.meshletInfo.AABBMin;
				meshletInfo.AABBMax = meshlet.meshletInfo.AABBMax;
				meshletInfo.vertexCount = meshlet.uniqueIndex.size(); // (x,y,z,w,uv1X....)
				meshletInfo.primitiveCount = meshlet.primitives.size() / 3; // 三角形
				meshletInfo.vertexDataOffset = m_vertexData.size() * (sizeof(VertexData) / sizeof(float));
				meshletInfo.uniqueIndexOffset = m_uniqueIndexData.size();
				meshletInfo.primitivesOffset = m_primitivesData.size();
				m_meshletInfo.push_back(meshletInfo);

				// uniqueIndex
				m_uniqueIndexData.insert(m_uniqueIndexData.end(), meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end());

				// primitive
				m_primitivesData.insert(m_primitivesData.end(), meshlet.primitives.begin(), meshlet.primitives.end());
			}

			// VertexData
			m_vertexData.insert(m_vertexData.end(), mesh.vertex.begin(), mesh.vertex.end());
			meshInfo.meshletCount = static_cast<uint32_t>(m_meshletInfo.size()) - meshInfo.meshletInfoOffset; // MeshMetaDataの個数
			m_meshInfo.push_back(meshInfo);

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].rebuild = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].rebuild = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].rebuild = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].rebuild = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].rebuild = true;

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].update = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].update = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].update = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].update = true;
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].update = true;

			meshIDs.push_back(meshInfo.meshID);
		}
		m_customMeshIDTable.emplace(name, meshIDs);
		return meshIDs;
	}

	void MeshInfoDescriptorSet::updateMeshInfo()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_meshInfo.data(), sizeof(MeshInfo) * m_meshInfo.size());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshInfo)].update = false;
	}

	void MeshInfoDescriptorSet::updateMeshletInfo()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_meshletInfo.data(), sizeof(MeshletInfo) * m_meshletInfo.size());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::MeshletInfo)].update = false;
	}

	void MeshInfoDescriptorSet::updateVertexData()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_vertexData.data(), sizeof(VertexData) * m_vertexData.size());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VertexData)].update = false;
	}

	void MeshInfoDescriptorSet::updateUniqueIndexData()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_uniqueIndexData.data(), sizeof(uint32_t) * m_uniqueIndexData.size());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::UniqueIndexData)].update = false;
	}

	void MeshInfoDescriptorSet::updatePrimitivesData()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_primitivesData.data(), sizeof(uint32_t) * m_primitivesData.size());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::PrimitivesData)].update = false;
	}
}