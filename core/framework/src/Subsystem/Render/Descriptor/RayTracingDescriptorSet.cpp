// Copyright (c) 2025 kong9812
#include "RayTracingDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	RayTracingDescriptorSet::RayTracingDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_tlas(VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
		{
			Descriptor descriptor{};
			descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
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
	}

	void RayTracingDescriptorSet::SetTLAS(const VkAccelerationStructureKHR& tlas)
	{
		m_tlas = tlas;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TLAS)].rebuild = true;
	}
}