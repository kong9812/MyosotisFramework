// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "iimgui.h"
#include "ClassPointer.h"
#include "UpdateData.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderSwapchain;
	TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);

	class EditorGUI
	{
	public:
		EditorGUI(
			const VkInstance& instance,
			const RenderDevice_ptr renderDevice,
			const VkRenderPass& renderPass,
			const RenderSwapchain_ptr renderSwapchain);
		~EditorGUI();

		void Update(const UpdateData& updateData);
	private:
		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorGUI);
}