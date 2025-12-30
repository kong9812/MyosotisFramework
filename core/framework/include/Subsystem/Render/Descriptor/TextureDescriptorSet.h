// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <queue>
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
		void UpdateImage(const uint32_t imageID, const DescriptorBindingIndex type, const VkDescriptorImageInfo imageInfo);
		void DeleteImage(const uint32_t imageID, const DescriptorBindingIndex type);
		void Update() override;


		void SetDummySampled2D(const VkDescriptorImageInfo dummy) { m_dummySampled2D = dummy; }
		void SetDummyStorage2D(const VkDescriptorImageInfo dummy) { m_dummyStorage2D = dummy; }

	private:
		void updateCombinedImageSampler();
		void updateStorageImage();

		std::vector<VkDescriptorImageInfo> m_combinedImageSamplerImageInfo;
		std::queue<uint32_t> m_freeCombinedImageSamplerImageList;
		std::vector<VkDescriptorImageInfo> m_storageImageInfo;
		std::queue<uint32_t> m_freeStorageImageList;

		VkDescriptorImageInfo m_dummySampled2D;
		VkDescriptorImageInfo m_dummyStorage2D;
	};

	TYPEDEF_SHARED_PTR_ARGS(TextureDescriptorSet);
}