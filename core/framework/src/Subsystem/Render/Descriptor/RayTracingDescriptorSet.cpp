// Copyright (c) 2025 kong9812
#include "RayTracingDescriptorSet.h"
#include "RenderDevice.h"

namespace MyosotisFW::System::Render
{
	RayTracingDescriptorSet::RayTracingDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_tlas(VK_NULL_HANDLE)
	{
		Descriptor descriptor{};
		descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		descriptor.descriptorCount = 1;
		descriptor.rebuild = false;
		descriptor.update = false;
		m_descriptors.push_back(descriptor);

		for (uint32_t i = static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo); i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
		{
			Descriptor descriptor{};
			descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			descriptor.descriptorCount = 1;
			descriptor.rebuild = false;
			descriptor.update = false;
			m_descriptors.push_back(descriptor);
		}
		createDescriptorSet();
	}

	void RayTracingDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLAS)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(VkAccelerationStructureKHR));
			buildASDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::TLAS), { m_tlas });
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLAS)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].rebuild)
		{
			size_t size = sizeof(TLASInstanceInfo) * m_tlasInstanceInfo.size();
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].rebuild = false;
		}

		// SSBO/UBO更新
		updateTLASInstanceInfo();
	}

	void RayTracingDescriptorSet::SetTLAS(const VkAccelerationStructureKHR& tlas)
	{
		m_tlas = tlas;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLAS)].rebuild = true;
	}

	uint32_t RayTracingDescriptorSet::AddTLASInstanceInfo(TLASInstanceInfo tlasInstanceInfo)
	{
		uint32_t index = static_cast<uint32_t>(m_tlasInstanceInfo.size());
		m_tlasInstanceInfo.push_back(std::move(tlasInstanceInfo));
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].update = true;
		return index;
	}

	void RayTracingDescriptorSet::updateTLASInstanceInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].update)
		{
			vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(),
				m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].buffer,
				m_tlasInstanceInfo.data(),
				sizeof(TLASInstanceInfo) * m_tlasInstanceInfo.size());

			// 一時データクリア
			m_tlasInstanceInfo.clear();

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLASInstanceInfo)].update = false;
		}
	}
}