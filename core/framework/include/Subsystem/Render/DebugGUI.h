// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "iimgui.h"
#include "ClassPointer.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderSwapchain;
	TYPEDEF_SHARED_PTR_FWD(RenderSwapchain);

	class DebugGUI
	{
	public:
		DebugGUI(
			//GLFWwindow& glfwWindow,
			VkInstance& instance,
			RenderDevice_ptr renderDevice,
			VkRenderPass& renderPass,
			RenderSwapchain_ptr renderSwapchain,
			VkPipelineCache& pipelineCache);
		~DebugGUI();

		void Update(UpdateData updateData);

		void BeginDebugCommandBuffer();
		void BindDebugGUIElement() const;
		void EndDebugCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		float m_deltaTime;	// fpsカウンター
		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
	};
	TYPEDEF_SHARED_PTR_ARGS(DebugGUI);
}