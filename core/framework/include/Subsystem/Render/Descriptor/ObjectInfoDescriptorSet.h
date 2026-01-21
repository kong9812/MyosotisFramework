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
			FalseNegativeVBDispatchInfoIndex,	// phase2用
			Count
		};

		const VBDispatchInfo* GetVBDispatchInfo(const uint32_t index) const { return m_vbDispatchInfo.size() > index ? &m_vbDispatchInfo[index] : nullptr; }
		void ResetVBDispatchInfo() { m_vbDispatchInfo.clear(); }
		void AddObjectInfo(const ObjectInfo& objectInfo, std::vector<VBDispatchInfo> vbDispatchInfo);
		uint32_t GetFalseNegativeVBDispatchInfoIndexCount();
		void Update() override;

	private:
		void updateObjectInfo();
		void updateVBDispatchInfo();

		std::vector<ObjectInfo> m_objectInfo;
		std::vector<VBDispatchInfo> m_vbDispatchInfo;
	};

	TYPEDEF_SHARED_PTR_ARGS(ObjectInfoDescriptorSet);
}