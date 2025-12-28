// Copyright (c) 2025 kong9812
#include "ObjectInfoDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	ObjectInfoDescriptorSet::ObjectInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_objectInfo(),
		m_vbDispatchInfo()
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

	// 毎フレーム更新!
	void ObjectInfoDescriptorSet::AddObjectInfo(const ObjectInfo& objectInfo, std::vector<VBDispatchInfo> vbDispatchInfo)
	{
		m_objectInfo.push_back(objectInfo);
		m_vbDispatchInfo.insert(m_vbDispatchInfo.end(), vbDispatchInfo.begin(), vbDispatchInfo.end());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].update = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::FalseNegativeVBDispatchInfoIndex)].update = true;
	}

	uint32_t ObjectInfoDescriptorSet::GetFalseNegativeVBDispatchInfoIndexCount()
	{
		uint32_t count = 0;
		memcpy(&count, m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::FalseNegativeVBDispatchInfoIndex)].buffer.allocationInfo.pMappedData, sizeof(uint32_t));
		return count;
	}

	void ObjectInfoDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(ObjectInfo)) * static_cast<uint32_t>(m_objectInfo.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(VBDispatchInfo)) * static_cast<uint32_t>(m_vbDispatchInfo.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::FalseNegativeVBDispatchInfoIndex)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(uint32_t)) + static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_vbDispatchInfo.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::FalseNegativeVBDispatchInfoIndex), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::FalseNegativeVBDispatchInfoIndex)].rebuild = false;
		}

		// SSBO/UBO更新
		updateObjectInfo();
		updateVBDispatchInfo();
	}

	void ObjectInfoDescriptorSet::updateObjectInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].update)
		{
			vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(),
				m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].buffer,
				m_objectInfo.data(),
				sizeof(ObjectInfo) * m_objectInfo.size());

			// 一時データクリア
			m_objectInfo.clear();

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].update = false;
		}
	}

	void ObjectInfoDescriptorSet::updateVBDispatchInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].update)
		{
			vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(),
				m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].buffer,
				m_vbDispatchInfo.data(),
				sizeof(VBDispatchInfo) * m_vbDispatchInfo.size());

			// 一時データクリア
			m_vbDispatchInfo.clear();

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].update = false;
		}
	}
}