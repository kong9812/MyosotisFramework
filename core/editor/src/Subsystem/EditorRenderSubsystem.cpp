// Copyright (c) 2025 kong9812
#include "EditorRenderSubsystem.h"
#include "EditorRenderResources.h"

#include "EditorFinalCompositionRenderPass.h"
#include "EditorFinalCompositionRenderPipeline.h"
#include "EditorCamera.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorRenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(instance, surface);

		// EditorGUIの初期化
		m_editorGUI = CreateEditorGUIPointer(instance, m_device, m_graphicsQueue, m_editorRenderPass->GetRenderPass(), m_swapchain);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer();
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
		if (m_selectedObject)
		{
			ImGui::Text("Selected Object: %s", m_selectedObject->GetName().c_str());
		}
		ImGui::End();

		__super::Update(updateData);
	}

	void EditorRenderSubsystem::EditorRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		m_editorRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currentCommandBuffer);
		m_editorRenderPass->EndRender(currentCommandBuffer);
	}

	void EditorRenderSubsystem::ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY)
	{
		VK_VALIDATION(vkQueueWaitIdle(m_graphicsQueue));
		uint32_t pixelSize = 16;	// RGBA32

		// 1 pixel only
		VkBufferImageCopy bufferImageCopy = Utility::Vulkan::CreateInfo::bufferImageCopy(1, 1);
		bufferImageCopy.imageOffset = { cursorPosX, cursorPosY, 0 };

		// create command buffer
		VkCommandBuffer commandBuffer{};
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_transferCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &commandBufferAllocateInfo, &commandBuffer));

		// begin command buffer
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		// copy image
		Buffer stagingBuffer{};
		{// CPU buffer (staging buffer)
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(pixelSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		}
		vkCmdCopyImageToBuffer(commandBuffer, m_resources->GetNormal().image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer.buffer, 1, &bufferImageCopy);

		// end command buffer
		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

		// submit info
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		VK_VALIDATION(vkQueueSubmit(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_VALIDATION(vkQueueWaitIdle(m_transferQueue));

		void* data{};
		float id = 0.0f;
		VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), stagingBuffer.allocation, &data));
		memcpy(&id, &static_cast<float*>(data)[3], sizeof(float));
		vmaUnmapMemory(m_device->GetVmaAllocator(), stagingBuffer.allocation);

		// clean up
		vkFreeCommandBuffers(*m_device, m_transferCommandPool, 1, &commandBuffer);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);

		uint32_t idx = static_cast<uint32_t>(id * static_cast<float>(AppInfo::g_maxObject));
		if (idx != 1)
		{
			bool test = false;
		}
		if (idx != 2)
		{
			bool test = false;
		}
		m_selectedObject = m_objects[idx];
	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device);
		m_resources->Initialize(m_swapchain->GetWidth(), m_swapchain->GetHeight());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		m_shadowMapRenderPass = CreateShadowMapRenderPassPointer(m_device, m_resources, AppInfo::g_shadowMapSize, AppInfo::g_shadowMapSize);
		m_shadowMapRenderPass->Initialize();

		m_mainRenderPass = CreateMainRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_mainRenderPass->Initialize();

		m_finalCompositionRenderPass = CreateEditorFinalCompositionRenderPassPointer(m_device, std::dynamic_pointer_cast<EditorRenderResources>(m_resources), m_swapchain);
		m_finalCompositionRenderPass->Initialize();

		m_editorRenderPass = CreateEditorRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_editorRenderPass->Initialize();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		m_shadowMapRenderPipeline = CreateShadowMapRenderPipelinePointer(m_device);
		m_shadowMapRenderPipeline->Initialize(m_resources, m_shadowMapRenderPass->GetRenderPass());

		m_skyboxRenderPipeline = CreateSkyboxRenderPipelinePointer(m_device);
		m_skyboxRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_deferredRenderPipeline = CreateDeferredRenderPipelinePointer(m_device);
		m_deferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_lightingRenderPipeline = CreateLightingRenderPipelinePointer(m_device);
		m_lightingRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_compositionRenderPipeline = CreateCompositionRenderPipelinePointer(m_device);
		m_compositionRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_interiorObjectDeferredRenderPipeline = CreateInteriorObjectDeferredRenderPipelinePointer(m_device);
		m_interiorObjectDeferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_finalCompositionRenderPipeline = CreateEditorFinalCompositionRenderPipelinePointer(m_device);
		m_finalCompositionRenderPipeline->Initialize(m_resources, m_finalCompositionRenderPass->GetRenderPass());
		m_lightingRenderPipeline->UpdateDirectionalLightInfo(m_shadowMapRenderPipeline->GetDirectionalLightInfo());

		m_lightingRenderPipeline->CreateShaderObject(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		m_compositionRenderPipeline->CreateShaderObject();
		m_finalCompositionRenderPipeline->CreateShaderObject();
	}

	void EditorRenderSubsystem::resizeRenderPass(const uint32_t& width, const uint32_t& height)
	{
		m_shadowMapRenderPass->Resize(width, height);
		m_mainRenderPass->Resize(width, height);
		m_finalCompositionRenderPass->Resize(width, height);
		m_editorRenderPass->Resize(width, height);
	}

	void EditorRenderSubsystem::resizeRenderPipeline()
	{
		// pipeline
		m_skyboxRenderPipeline->Resize(m_resources);
		m_shadowMapRenderPipeline->Resize(m_resources);
		m_deferredRenderPipeline->Resize(m_resources);
		m_lightingRenderPipeline->Resize(m_resources);
		m_compositionRenderPipeline->Resize(m_resources);
		m_finalCompositionRenderPipeline->Resize(m_resources);
		m_interiorObjectDeferredRenderPipeline->Resize(m_resources);

		// update descriptors
		m_lightingRenderPipeline->UpdateDescriptors(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		for (ObjectBase_ptr& object : m_objects)
		{
			if (IsStaticMesh(object->GetObjectType()))
			{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(object);
				StaticMeshShaderObject& shadowObject = staticMesh->GetStaticMeshShaderObject();
				m_shadowMapRenderPipeline->UpdateDescriptors(shadowObject);
			}
		}
	}
}