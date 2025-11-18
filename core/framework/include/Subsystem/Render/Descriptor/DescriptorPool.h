// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class DescriptorPool
	{
	public:
		DescriptorPool(const RenderDevice_ptr& device);
		~DescriptorPool();

		VkDescriptorPool& GetDescriptorPool() { return m_descriptorPool; }

	private:
		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
	};

	TYPEDEF_SHARED_PTR_ARGS(DescriptorPool);
}