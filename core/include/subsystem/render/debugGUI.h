// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "iimgui.h"
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

		void Update(Utility::Vulkan::Struct::UpdateData updateData);

		void BuildCommandBuffer(VkCommandBuffer& commandBuffer) const;

	private:
		float m_deltaTime;	// fpsカウンター
		RenderDevice_ptr m_device;
		VkDescriptorPool m_descriptorPool;
	};
	TYPEDEF_SHARED_PTR_ARGS(DebugGUI)
}