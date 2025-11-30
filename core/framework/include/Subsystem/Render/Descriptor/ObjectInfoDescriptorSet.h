// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "ObjectInfo.h"
#include "VBDispatchInfo.h"

namespace MyosotisFW::System::Render
{
	class ObjectInfoDescriptorSet : public DescriptorSetBase
	{
	public:
		ObjectInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~ObjectInfoDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			ObjectInfo = 0,
			VBDispatchInfo,
			Count
		};

		void AddObjectInfo(const ObjectInfo& objectInfo, std::vector<VBDispatchInfo> vbDispatchInfo);
		void Update() override;

	private:
		void updateObjectInfo();
		void updateVBDispatchInfo();

		std::vector<ObjectInfo> m_objectInfo;
		std::vector<VBDispatchInfo> m_vbDispatchInfo;
	};

	TYPEDEF_SHARED_PTR_ARGS(ObjectInfoDescriptorSet);
}