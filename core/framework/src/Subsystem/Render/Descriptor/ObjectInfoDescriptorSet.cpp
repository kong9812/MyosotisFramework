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
	void ObjectInfoDescriptorSet::AddObjectInfo(const ObjectInfo& objectInfo)
	{
		m_objectInfo.push_back(objectInfo);
	}

	void ObjectInfoDescriptorSet::AddVBDispatchInfo(const VBDispatchInfo& vbDispatchInfo)
	{
		m_vbDispatchInfo.push_back(vbDispatchInfo);
	}

	// オブジェクト追加/削除の時に呼ぶ!
	void ObjectInfoDescriptorSet::DescriptorSetRebuildRequest()
	{
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].rebuild = true;
	}

	void ObjectInfoDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(ObjectInfo)) * static_cast<uint32_t>(m_objectInfo.size());
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(VBDispatchInfo)) * static_cast<uint32_t>(m_vbDispatchInfo.size());
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].rebuild = false;
		}

		// SSBO/UBO更新
		updateObjectInfo();
		updateVBDispatchInfo();
	}

	void ObjectInfoDescriptorSet::updateObjectInfo()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ObjectInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_objectInfo.data(), sizeof(ObjectInfo) * m_objectInfo.size());

		// 一時データクリア
		m_objectInfo.clear();
	}

	void ObjectInfoDescriptorSet::updateVBDispatchInfo()
	{
		// 可変部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::VBDispatchInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, m_vbDispatchInfo.data(), sizeof(VBDispatchInfo) * m_vbDispatchInfo.size());

		// 一時データクリア
		m_vbDispatchInfo.clear();
	}
}