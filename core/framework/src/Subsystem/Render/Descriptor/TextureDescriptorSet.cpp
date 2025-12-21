// Copyright (c) 2025 kong9812
#include "TextureDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	TextureDescriptorSet::TextureDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_combinedImageSamplerImageInfo(),
		m_storageImageInfo()
	{
		Descriptor descriptor{};
		descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		descriptor.descriptorCount = m_device->GetMaxDescriptorSetSampledImages();
		descriptor.rebuild = true;
		descriptor.update = true;
		m_descriptors.push_back(descriptor);
		descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		descriptor.descriptorCount = m_device->GetMaxDescriptorSetStorageImages();
		descriptor.rebuild = true;
		descriptor.update = true;
		m_descriptors.push_back(descriptor);

		createDescriptorSet();
	}

	uint32_t TextureDescriptorSet::AddImage(const DescriptorBindingIndex& type, VkDescriptorImageInfo imageInfo)
	{
		uint32_t index = 0;
		switch (type)
		{
		case DescriptorBindingIndex::CombinedImageSampler:
			index = static_cast<uint32_t>(m_combinedImageSamplerImageInfo.size());
			m_combinedImageSamplerImageInfo.push_back(imageInfo);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CombinedImageSampler)].update = true;
			break;
		case DescriptorBindingIndex::StorageImage:
			index = static_cast<uint32_t>(m_storageImageInfo.size());
			m_storageImageInfo.push_back(imageInfo);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::StorageImage)].update = true;
			break;
		default:
			break;
		}
		return index;
	}

	void TextureDescriptorSet::Update()
	{
		// SSBO/UBO更新
		updateCombinedImageSampler();
		updateStorageImage();
	}

	void TextureDescriptorSet::updateCombinedImageSampler()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CombinedImageSampler)].update)
		{
			VkWriteDescriptorSet writeDescriptorSet = Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
				static_cast<uint32_t>(DescriptorBindingIndex::CombinedImageSampler),
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_combinedImageSamplerImageInfo.data(),
				static_cast<uint32_t>(m_combinedImageSamplerImageInfo.size()));

			vkUpdateDescriptorSets(*m_device, 1, &writeDescriptorSet, 0, nullptr);

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CombinedImageSampler)].update = false;
		}
	}

	void TextureDescriptorSet::updateStorageImage()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::StorageImage)].update)
		{
			if (m_storageImageInfo.size() > 0)
			{
				VkWriteDescriptorSet writeDescriptorSet = Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
					static_cast<uint32_t>(DescriptorBindingIndex::StorageImage),
					VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_storageImageInfo.data(),
					static_cast<uint32_t>(m_storageImageInfo.size()));

				vkUpdateDescriptorSets(*m_device, 1, &writeDescriptorSet, 0, nullptr);
				m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::StorageImage)].update = false;
			}
		}
	}
}