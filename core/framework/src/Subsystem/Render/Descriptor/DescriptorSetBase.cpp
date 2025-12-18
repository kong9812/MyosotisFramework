// Copyright (c) 2025 kong9812
#include "DescriptorSetBase.h"
#include "RenderDevice.h"

namespace MyosotisFW::System::Render
{
	DescriptorSetBase::~DescriptorSetBase()
	{
		for (Descriptor& descriptor : m_descriptors)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), descriptor.buffer.buffer, descriptor.buffer.allocation);
		}
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
	}

	void DescriptorSetBase::createDescriptorSet()
	{
		// SetLayoutの作成
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding{};
		std::vector<VkDescriptorBindingFlags> descriptorBindingFlags{};
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_descriptors.size()); i++)
		{
			const Descriptor& descriptor = m_descriptors[i];
			descriptorSetLayoutBinding.push_back(
				Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(
					i,
					descriptor.descriptorType,
					descriptor.shaderStageFlagBits,
					descriptor.descriptorCount));
			// 未使用許可 & バインド後更新 を有効化
			descriptorBindingFlags.push_back(descriptor.descriptorBindingFlags);
		}
		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{};
		descriptorSetLayoutBindingFlagsCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size());
		descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(descriptorSetLayoutBinding);
		descriptorSetLayoutCreateInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_descriptorSet));
	}

	void DescriptorSetBase::buildSSBODescriptor(const uint32_t index, const uint32_t dataSize)
	{
		if (dataSize > 0)
		{
			Descriptor& descriptor = m_descriptors[index];
			// SSBOの作成
			descriptor.buffer = vmaTools::CreateShaderStorageBuffer(m_device->GetVmaAllocator(), dataSize);

			VkDescriptorBufferInfo vertexMetaDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(descriptor.buffer.buffer);
			VkWriteDescriptorSet writeDescriptorSet = Utility::Vulkan::CreateInfo::writeDescriptorSet(
				m_descriptorSet,
				index,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				&vertexMetaDataDescriptorBufferInfo);

			vkUpdateDescriptorSets(*m_device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}

	void DescriptorSetBase::buildASDescriptor(const uint32_t index, const std::vector<VkAccelerationStructureKHR>& handle)
	{
		Descriptor& descriptor = m_descriptors[index];
		VkWriteDescriptorSetAccelerationStructureKHR descriptorASInfo{};
		descriptorASInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorASInfo.accelerationStructureCount = static_cast<uint32_t>(handle.size());
		descriptorASInfo.pAccelerationStructures = handle.data();

		VkDescriptorBufferInfo* nullBufferInfo = nullptr;
		VkWriteDescriptorSet writeDescriptorSet = Utility::Vulkan::CreateInfo::writeDescriptorSet(
			m_descriptorSet,
			index,
			VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			nullBufferInfo);
		writeDescriptorSet.pNext = &descriptorASInfo;

		vkUpdateDescriptorSets(*m_device, 1, &writeDescriptorSet, 0, nullptr);
	}
}