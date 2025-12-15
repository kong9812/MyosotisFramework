// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "CameraInfo.h"
#include "SceneInfo.h"
#include "ScreenInfo.h"
#include "VBDispatchInfo.h"

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
			Count
		};

		void Update() override;
		void SetTLAS(const VkAccelerationStructureKHR& tlas);

	private:
		VkAccelerationStructureKHR m_tlas;
	};

	TYPEDEF_SHARED_PTR_ARGS(RayTracingDescriptorSet);
}