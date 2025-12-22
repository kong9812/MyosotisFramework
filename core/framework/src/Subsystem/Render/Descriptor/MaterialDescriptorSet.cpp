// Copyright (c) 2025 kong9812
#include "MaterialDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	MaterialDescriptorSet::MaterialDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_basicMaterial()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
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

	void MaterialDescriptorSet::AddBasicMaterial(const BasicMaterialsHandle& basicMaterialsHandle)
	{
		for (const BasicMaterialHandle& materialHandle : basicMaterialsHandle)
		{
			std::shared_ptr<const BasicMaterial> material = materialHandle.lock();
			m_basicMaterial.push_back(*material);
		}
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].rebuild = true;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].update = true;
	}

	void MaterialDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(BasicMaterial)) * static_cast<uint32_t>(m_basicMaterial.size());
			buildSSBODescriptor(static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].rebuild = false;
		}

		// SSBO/UBO更新
		updateBasicMaterial();
	}

	void MaterialDescriptorSet::updateBasicMaterial()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].update)
		{
			vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(),
				m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].buffer,
				m_basicMaterial.data(),
				sizeof(BasicMaterial) * m_basicMaterial.size());

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::BasicMaterial)].update = false;
		}
	}
}