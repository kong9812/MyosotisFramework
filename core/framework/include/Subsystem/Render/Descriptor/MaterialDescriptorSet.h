// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "BasicMaterial.h"

namespace MyosotisFW::System::Render
{
	class MaterialDescriptorSet : public DescriptorSetBase
	{
	public:
		MaterialDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~MaterialDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			BasicMaterialInfo = 0,
			Count
		};

		void AddBasicMaterial(const BasicMaterialsHandle& basicMaterialsHandle);
		void Update() override;

	private:
		void updateBasicMaterial();

		std::vector<BasicMaterialInfo> m_basicMaterial;
	};

	TYPEDEF_SHARED_PTR_ARGS(MaterialDescriptorSet);
}