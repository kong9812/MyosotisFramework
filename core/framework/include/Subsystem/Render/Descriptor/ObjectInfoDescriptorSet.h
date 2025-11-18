// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "ObjectInfo.h"

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
			Count
		};

		void AddObjectInfo(const ObjectInfo& objectInfo);
		void DescriptorSetRebuildRequest();
		void Update() override;

	private:
		void updateObjectInfo();

		std::vector<ObjectInfo> m_objectInfo;
	};

	TYPEDEF_SHARED_PTR_ARGS(ObjectInfoDescriptorSet);
}