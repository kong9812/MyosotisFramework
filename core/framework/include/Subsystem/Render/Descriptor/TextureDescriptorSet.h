// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "ObjectInfo.h"
#include "Image.h"

namespace MyosotisFW::System::Render
{
	class TextureDescriptorSet : public DescriptorSetBase
	{
	public:
		TextureDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~TextureDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			CombinedImageSampler = 0,
			StorageImage,
			Count
		};

		uint32_t AddImage(const DescriptorBindingIndex type, const VkDescriptorImageInfo imageInfo);
		void Update() override;

	private:
		void updateCombinedImageSampler();
		void updateStorageImage();

		std::vector<VkDescriptorImageInfo> m_combinedImageSamplerImageInfo;
		std::vector<VkDescriptorImageInfo> m_storageImageInfo;
	};

	TYPEDEF_SHARED_PTR_ARGS(TextureDescriptorSet);
}