// Copyright (c) 2025 kong9812
#include "EditorRenderSubsystem.h"

#include "MObject.h"

#include "EditorRenderResources.h"
#include "RenderSwapchain.h"
#include "RenderDevice.h"
#include "RenderQueue.h"

#include "EditorGUI.h"
#include "EditorCamera.h"
#include "StaticMesh.h"

#include "VisibilityBufferRenderPass.h"

#include "VisibilityBufferPipeline.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorRenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(instance, surface);

		// EditorGUIの初期化
		//m_editorGUI = CreateEditorGUIPointer(instance, m_device, m_editorRenderPass->GetRenderPass(), m_swapchain);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer(0);
		m_mainCamera->UpdateScreenSize(glm::vec2(static_cast<float>(m_swapchain->GetWidth()), static_cast<float>(m_swapchain->GetHeight())));
	}

	void EditorRenderSubsystem::Update(const UpdateData& updateData)
	{
		m_editorGUI->Update(updateData);
		//ImGui::ShowDemoWindow();

		ImGui::Begin("MainEditorWindow");
#ifdef DEBUG
		ImGui::Text("Model: Debug");
#elif FWDLL
		ImGui::Text("Model: DLL\nF5: Hot reload");
#elif RELEASE
		ImGui::Text("Model: Release");
#endif
		ImGui::Text("FPS: %.2f(%.2fms)", (1.0f / updateData.deltaTime), updateData.deltaTime * 1000.0f);
		ImGui::Text("Mouse Pos: %.2f %.2f", updateData.mousePos.x, updateData.mousePos.y);
		ImGui::Text("Pressed key count (Keyboard): %d", updateData.keyActions.size());
		ImGui::Text("Pressed key count (Mouse): %d", updateData.mouseButtonActions.size());
		if (m_selectedObject.Get())
		{
			ImGui::Text("Selected Object: %s", m_selectedObject.Get()->GetName().c_str());
		}
		ImGui::End();

		__super::Update(updateData);
	}

	void EditorRenderSubsystem::EditorRender()
	{
		VkCommandBuffer currentCommandBuffer = m_device->GetGraphicsQueue()->GetCommandBuffer(m_currentBufferIndex);
		//m_editorRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);
		//ImGui::Render();
		//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currentCommandBuffer);
		//m_editorRenderPass->EndRender(currentCommandBuffer);
	}

	void EditorRenderSubsystem::ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY)
	{
		//if (static_cast<uint32_t>(m_objects.size()) == 0) return;

		//RenderQueue_ptr transferQueue = m_device->GetTransferQueue();

		//// ObjectSelectFence
		//VkFence fence = VK_NULL_HANDLE;
		//VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		//VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

		//// 1 pixel only
		//VkBufferImageCopy bufferImageCopy = Utility::Vulkan::CreateInfo::bufferImageCopy(1, 1);
		//bufferImageCopy.imageOffset = { cursorPosX, cursorPosY, 0 };

		//// create command buffer
		//VkCommandBuffer commandBuffer{};
		//VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_transferCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		//VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, &commandBuffer));

		//// begin command buffer
		//VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		//VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		//// copy image
		//Buffer stagingBuffer{};
		//{// CPU buffer (staging buffer)
		//	VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(static_cast<uint32_t>(sizeof(uint32_t)), VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		//	VmaAllocationCreateInfo allocationCreateInfo{};
		//	allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
		//	VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		//}
		//vkCmdCopyImageToBuffer(commandBuffer, m_resources->GetIdMap().image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer.buffer, 1, &bufferImageCopy);

		//// end command buffer
		//VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

		//// submit info
		//VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
		//submitInfo.commandBufferCount = 1;
		//submitInfo.pCommandBuffers = &commandBuffer;
		//VK_VALIDATION(vkWaitForFences(*m_device, 1, &m_renderFence, VK_TRUE, UINT64_MAX));
		//transferQueue->Submit(submitInfo, fence);
		//VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

		//void* data{};
		//uint32_t id = 0;
		//VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), stagingBuffer.allocation, &data));
		//memcpy(&id, data, sizeof(uint32_t));
		//vmaUnmapMemory(m_device->GetVmaAllocator(), stagingBuffer.allocation);

		//// clean up
		//vkFreeCommandBuffers(*m_device, m_transferCommandPool, 1, &commandBuffer);
		//vmaDestroyBuffer(m_device->GetVmaAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
		//vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		//m_selectedObject.Set(m_objects[id]);
	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device, m_renderDescriptors);
		m_resources->Initialize(m_swapchain->GetWidth(), m_swapchain->GetHeight());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		m_visibilityBufferRenderPass = CreateVisibilityBufferRenderPassPointer(m_device, m_resources, m_swapchain);
		m_visibilityBufferRenderPass->Initialize();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		m_visibilityBufferPipeline = CreateVisibilityBufferPipelinePointer(m_device, m_renderDescriptors);
		m_visibilityBufferPipeline->Initialize(m_resources, m_visibilityBufferRenderPass->GetRenderPass());
		//m_visibilityBufferRenderPhase2Pipeline = CreateVisibilityBufferRenderPhase2PipelinePointer(m_device, m_renderDescriptors);
		//m_visibilityBufferRenderPhase2Pipeline->Initialize(m_resources, m_visibilityBufferRenderPass->GetRenderPass());
	}

	void EditorRenderSubsystem::resizeRenderPass(const uint32_t width, const uint32_t height)
	{
		//m_editorRenderPass->Resize(width, height);
	}
}