// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "iimgui.h"
#include "ClassPointer.h"

#include "RenderDevice.h"
#include "RenderSwapchain.h"

namespace MyosotisFW::System::Render
{
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
	TYPEDEF_SHARED_PTR_ARGS(DebugGUI)
}