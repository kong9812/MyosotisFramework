// Copyright (c) 2025 kong9812
#include "EditorRenderSubsystem.h"

#include "iglm.h"
#include "MObject.h"

#include "EditorRenderResources.h"
#include "RenderSwapchain.h"
#include "RenderDevice.h"
#include "RenderQueue.h"
#include "MObjectRegistry.h"

#include "EditorCamera.h"
#include "StaticMesh.h"

#include "Gizmo.h"
#include "GizmoRenderPass.h"
#include "GizmoPipeline.h"

#include "GridRenderPass.h"
#include "GridPipeline.h"

#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	EditorRenderSubsystem::~EditorRenderSubsystem()
	{
		m_device->GetGraphicsQueue()->WaitIdle();
		m_device->GetComputeQueue()->WaitIdle();
		m_device->GetTransferQueue()->WaitIdle();

		vkDestroyCommandPool(*m_device, m_objectSelectCommandPool, m_device->GetAllocationCallbacks());
	}

	void EditorRenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(instance, surface);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer(0);
		m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
		m_mainCamera->SetMainCamera(true);
		m_renderDescriptors->GetSceneInfoDescriptorSet()->AddCamera(m_mainCamera);

		// ObjectSelectCommandPool
		initializeObjectSelectCommandPool();

		// Gizmo
		initializeGizmo();
	}

	void EditorRenderSubsystem::Update(const UpdateData& updateData)
	{
		__super::Update(updateData);
		m_gizmo->Update(updateData, m_mainCamera);

		if (m_mainCamera->IsMoved())
		{
			m_gizmo->SetSelectObject(nullptr);
		}
	}

	void EditorRenderSubsystem::EditorRender()
	{

	}

	void EditorRenderSubsystem::ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY)
	{
		if ((m_vbDispatchInfoCount <= 0) || (!m_mainCamera) || (cursorPosX > m_swapchain->GetScreenSize().x) || (cursorPosY > m_swapchain->GetScreenSize().y)) return;
		if (m_gizmo->IsHovered()) return;

		m_gizmo->SetSelectObject(nullptr);

		const uint32_t currentFrameIndex = m_frameCounter % AppInfo::g_maxInFlightFrameCount;
		const Image& targetVBImage = m_resources->GetVisibilityBuffer(currentFrameIndex);

		m_device->GetGraphicsQueue()->WaitIdle();

		// このFrameのVBufferPhase2が終わったら、VBを取得
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo(
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
			VK_NULL_HANDLE,		// wait
			VK_NULL_HANDLE);											// signal

		MyosotisFW::Buffer buffer = vmaTools::CreateBuffer(
			m_device->GetVmaAllocator(),
			sizeof(uint32_t),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY,
			VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT
		);

		VkCommandBuffer commandBuffer{};
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_objectSelectCommandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_VALIDATION(vkAllocateCommandBuffers(*m_device, &cmdBufAllocateInfo, &commandBuffer));
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		VkImageMemoryBarrier barrier{};
		{// VB -> TRANSFER_SRC
			barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = targetVBImage.image;
			barrier.subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkBufferImageCopy bufferImageCopy = Utility::Vulkan::CreateInfo::bufferImageCopy(1, 1, glm::ivec3(cursorPosX, cursorPosY, 0));
		vkCmdCopyImageToBuffer(commandBuffer, targetVBImage.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.buffer, 1, &bufferImageCopy);

		{// VB -> COLOR_ATTACHMENT
			barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = targetVBImage.image;
			barrier.subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		}
		vkCmdPipelineBarrier(commandBuffer,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();

		VkFence fence{};
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

		graphicsQueue->Submit(submitInfo, fence);
		VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

		if (buffer.needFlush)
		{
			VK_VALIDATION(vmaInvalidateAllocation(m_device->GetVmaAllocator(), buffer.allocation, 0, buffer.localSize));
		}

		uint32_t pixelValue = 0;
		if (buffer.allocationInfo.pMappedData)
		{
			pixelValue = *static_cast<uint32_t*>(buffer.allocationInfo.pMappedData);
		}
		else
		{
			// マップされていなかった場合
			void* mappedData = nullptr;
			vmaMapMemory(m_device->GetVmaAllocator(), buffer.allocation, &mappedData);
			pixelValue = *static_cast<uint32_t*>(mappedData);
			vmaUnmapMemory(m_device->GetVmaAllocator(), buffer.allocation);
		}

		// pixel value -> objectID
		if (pixelValue != 0)
		{
			const uint32_t PRIM_BITS = 7u;
			const uint32_t PRIM_MASK = (1u << PRIM_BITS) - 1u;

			uint32_t vbIndex = 0;
			uint32_t primIndex = 0;

			uint32_t packed = pixelValue - 1;

			primIndex = packed & PRIM_MASK;
			vbIndex = packed >> PRIM_BITS;

			const VBDispatchInfo* vbDispatchInfo = m_renderDescriptors->GetObjectInfoDescriptorSet()->GetVBDispatchInfo(vbIndex);
			if (vbDispatchInfo)
			{
				const MObject_ptr& obj = (*m_objects)[vbDispatchInfo->objectID];
				m_gizmo->SetSelectObject(obj);
				m_objectSelectedCallback(obj);
				Logger::Debug("Selected object: " + std::to_string(vbDispatchInfo->objectID));
			}
		}

		// 後片付け
		vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		vkFreeCommandBuffers(*m_device, m_objectSelectCommandPool, 1, &commandBuffer);
		vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
	}

	const MObject_ptr* EditorRenderSubsystem::GetMObjectPtr(const MObject* raw)
	{
		return m_objectRegistry->GetMObjectPtr(raw);
	}

	void EditorRenderSubsystem::ResetCamera()
	{
		if (!m_mainCamera) return;
		m_mainCamera->ResetCamera();
	}

	void EditorRenderSubsystem::SetCameraFocusObject(const MObject_ptr& object)
	{
		if (!m_mainCamera) return;

		if (object->GetMeshCount() == 0)
		{
			glm::vec3 cameraPos = m_mainCamera->GetCameraPos();
			m_mainCamera->SetFocusPosition(object->GetPos());
		}
		else
		{
			const glm::vec3 aabbMin = object->GetAABBMin() * glm::vec3(object->GetScale());
			const glm::vec3 aabbMax = object->GetAABBMax() * glm::vec3(object->GetScale());

			// 距離
			const glm::vec3 size = aabbMax - aabbMin;
			const float radius = glm::max(size.x, glm::max(size.y, size.x)) * 0.5f;
			const float offset = 1.5f;
			const float fov = glm::radians(m_mainCamera->GetCameraFOV());
			const float distance = (radius / glm::tan(fov * 0.5f)) + offset;

			// 座標
			const glm::vec3 aabbCenter = (aabbMax + aabbMin) * 0.5f;
			const glm::vec3 focusTargetPos = glm::vec3(object->GetPos()) + aabbCenter;

			m_mainCamera->SetFocusPosition(focusTargetPos, distance);
		}

		m_gizmo->SetSelectObject(object);
		m_objectSelectedCallback(object);
	}

	void EditorRenderSubsystem::render(const uint32_t currentFrameIndex)
	{
		__super::render(currentFrameIndex);

		VkCommandBuffer commandBuffer = m_commandBuffers.render[currentFrameIndex];
		{// Grid Render Pass
			VkDebugUtilsLabelEXT gridLabel = Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.0f, 0.25f, 5.0f), "Grid Render");
			m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &gridLabel);
			m_gridRenderPass->BeginRender(commandBuffer, currentFrameIndex);
			if (m_mainCamera)
			{
				m_gridPipeline->Update(m_mainCamera);
				m_gridPipeline->BindCommandBuffer(commandBuffer);
			}
			m_gridRenderPass->EndRender(commandBuffer);
			m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
		}
		{// Gizmo Render Pass
			if (m_gizmo->IsEnable())
			{
				VkDebugUtilsLabelEXT gizmoLabel = Utility::Vulkan::CreateInfo::debugUtilsLabelEXT(glm::vec3(0.0f, 0.25f, 5.0f), "Gizmo Render");
				m_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &gizmoLabel);
				m_gizmoRenderPass->BeginRender(commandBuffer, currentFrameIndex);
				if (m_mainCamera)
				{
					m_gizmoPipeline->BindCommandBuffer(commandBuffer, m_gizmo->GetGizmoAxisDrawCommand());
				}
				m_gizmoRenderPass->EndRender(commandBuffer);
				m_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
			}
		}
	}

	void EditorRenderSubsystem::initializeGizmo()
	{
		m_gizmo = CreateGizmoPointer();
		m_gizmo->SetObjectMovedCallback(m_objectMovedCallback);
	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device, m_renderDescriptors);
		m_resources->Initialize(m_swapchain->GetScreenSize());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		__super::initializeRenderPass();
		m_gizmoRenderPass = CreateGizmoRenderPassPointer(m_device, m_resources, m_swapchain);
		m_gizmoRenderPass->Initialize();

		m_gridRenderPass = CreateGridRenderPassPointer(m_device, m_resources, m_swapchain);
		m_gridRenderPass->Initialize();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		__super::initializeRenderPipeline();
		m_gizmoPipeline = CreateGizmoPipelinePointer(m_device, m_renderDescriptors);
		m_gizmoPipeline->Initialize(m_resources, m_gizmoRenderPass->GetRenderPass());

		m_gridPipeline = CreateGridPipelinePointer(m_device, m_renderDescriptors);
		m_gridPipeline->Initialize(m_resources, m_gridRenderPass->GetRenderPass());
	}

	void EditorRenderSubsystem::initializeObjectSelectCommandPool()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetGraphicsQueue()->GetQueueFamilyIndex());
		VK_VALIDATION(vkCreateCommandPool(*m_device, &cmdPoolInfo, m_device->GetAllocationCallbacks(), &m_objectSelectCommandPool));
	}

	void EditorRenderSubsystem::resizeRenderPass()
	{
		__super::resizeRenderPass();
		m_gizmoRenderPass->Resize(m_swapchain->GetScreenSize());
		m_gridRenderPass->Resize(m_swapchain->GetScreenSize());
	}

	void EditorRenderSubsystem::resizeRenderPipeline()
	{
		__super::resizeRenderPipeline();
		m_gizmoPipeline->Resize(m_resources);
		m_gridPipeline->Resize(m_resources);
	}
}