// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "Descriptor.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class DescriptorSetBase
	{
	public:
		DescriptorSetBase(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
			m_device(device),
			m_descriptorPool(descriptorPool),
			m_descriptorSetLayout(VK_NULL_HANDLE),
			m_descriptorSet(VK_NULL_HANDLE),
			m_descriptors() {
		}
		~DescriptorSetBase();

		virtual void Update() = 0;
		VkDescriptorSet GetDescriptorSet() const { return m_descriptorSet; }
		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

	protected:
		void createDescriptorSet();
		void buildSSBODescriptor(const uint32_t index, const uint32_t dataSize);
		void buildASDescriptor(const uint32_t index, const std::vector<VkAccelerationStructureKHR>& handle);

		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;
		std::vector<Descriptor> m_descriptors;
	};

	TYPEDEF_SHARED_PTR_ARGS(DescriptorSetBase);
}