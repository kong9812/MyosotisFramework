// Copyright (c) 2025 kong9812
#include "RenderDescriptors.h"
#include "RenderDevice.h"

namespace MyosotisFW::System::Render
{
	RenderDescriptors::RenderDescriptors(const RenderDevice_ptr& device)
	{
		m_descriptorPool = CreateDescriptorPoolPointer(device);

		m_sceneInfoDescriptorSet = CreateSceneInfoDescriptorSetPointer(device, m_descriptorPool->GetDescriptorPool());
		m_objectInfoDescriptorSet = CreateObjectInfoDescriptorSetPointer(device, m_descriptorPool->GetDescriptorPool());
		m_meshInfoDescriptorSet = CreateMeshInfoDescriptorSetPointer(device, m_descriptorPool->GetDescriptorPool());
		m_textureDescriptorSet = CreateTextureDescriptorSetPointer(device, m_descriptorPool->GetDescriptorPool());
		m_rayTracingDescriptorSet = CreateRayTracingDescriptorSetPointer(device, m_descriptorPool->GetDescriptorPool());
	}

	std::vector<VkDescriptorSet> RenderDescriptors::GetDescriptorSet()
	{
		std::vector<VkDescriptorSet> descriptorSets = {
			m_sceneInfoDescriptorSet->GetDescriptorSet(),
			m_objectInfoDescriptorSet->GetDescriptorSet(),
			m_meshInfoDescriptorSet->GetDescriptorSet(),
			m_textureDescriptorSet->GetDescriptorSet(),
			m_rayTracingDescriptorSet->GetDescriptorSet(),
		};
		return descriptorSets;
	}

	std::vector<VkDescriptorSetLayout> RenderDescriptors::GetDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			m_sceneInfoDescriptorSet->GetDescriptorSetLayout(),
			m_objectInfoDescriptorSet->GetDescriptorSetLayout(),
			m_meshInfoDescriptorSet->GetDescriptorSetLayout(),
			m_textureDescriptorSet->GetDescriptorSetLayout(),
			m_rayTracingDescriptorSet->GetDescriptorSetLayout()
		};
		return descriptorSetLayouts;
	}
}