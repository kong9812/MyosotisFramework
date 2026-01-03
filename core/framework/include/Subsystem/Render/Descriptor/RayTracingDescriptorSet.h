// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "TLASInstanceInfo.h"

namespace MyosotisFW::System::Render
{
	class RayTracingDescriptorSet : public DescriptorSetBase
	{
	public:
		RayTracingDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~RayTracingDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			TLAS,
			TLASInstanceInfo,
			Count
		};

		void Update() override;
		void SetTLAS(const VkAccelerationStructureKHR& tlas);
		uint32_t AddTLASInstanceInfo(TLASInstanceInfo tlasInstanceInfo);

	private:
		VkAccelerationStructureKHR m_tlas;
		std::vector<TLASInstanceInfo> m_tlasInstanceInfo;

		void updateTLASInstanceInfo();
	};

	TYPEDEF_SHARED_PTR_ARGS(RayTracingDescriptorSet);
}