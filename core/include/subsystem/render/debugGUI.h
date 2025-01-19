// Copyright (c) 2025 kong9812
#pragma once
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include "vulkan/vulkan.h"
#include "classPointer.h"

#include "renderDevice.h"
#include "renderSwapchain.h"

namespace MyosotisFW::System::Render
{
	class DebugGUI
	{
	public:
		DebugGUI(
			GLFWwindow& glfwWindow,
			VkInstance& instance,
			RenderDevice_ptr renderDevice,
			VkQueue& queue,
			VkRenderPass& renderPass,
			RenderSwapchain_ptr renderSwapchain,
			VkPipelineCache& pipelineCache);
		~DebugGUI();

		void BuildCommandBuffer(VkCommandBuffer& commandBuffer);

	private:
		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;

		bool m_mainWindow;
	};
	TYPEDEF_SHARED_PTR_ARGS(DebugGUI)
}