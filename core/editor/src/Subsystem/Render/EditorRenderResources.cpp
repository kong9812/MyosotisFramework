// Copyright (c) 2025 kong9812
#include "EditorRenderResources.h"

#include "RenderDevice.h"

#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	EditorRenderResources::~EditorRenderResources()
	{
		{// editor GUI render target
			vmaDestroyImage(m_device->GetVmaAllocator(), m_editorRenderTarget.image, m_editorRenderTarget.allocation);
			vkDestroyImageView(*m_device, m_editorRenderTarget.view, m_device->GetAllocationCallbacks());
		}
	}

	void EditorRenderResources::Initialize(const uint32_t width, const uint32_t height)
	{
		__super::Initialize(width, height);

		{// editor GUI render target
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfoForAttachment(AppInfo::g_surfaceFormat.format, width, height);
			VmaAllocationCreateInfo allocationCreateInfo{};
			VK_VALIDATION(vmaCreateImage(m_device->GetVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &m_editorRenderTarget.image, &m_editorRenderTarget.allocation, &m_editorRenderTarget.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfoForAttachment(m_editorRenderTarget.image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfo, m_device->GetAllocationCallbacks(), &m_editorRenderTarget.view));
		}
	}

	void EditorRenderResources::Resize(const uint32_t width, const uint32_t height)
	{
		{// attachment
			vmaDestroyImage(m_device->GetVmaAllocator(), m_editorRenderTarget.image, m_editorRenderTarget.allocation);
			vkDestroyImageView(*m_device, m_editorRenderTarget.view, m_device->GetAllocationCallbacks());
		}
		__super::Resize(width, height);
	}
}