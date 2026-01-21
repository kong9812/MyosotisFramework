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

#include "VisibilityBufferPhase1RenderPass.h"
#include "VisibilityBufferPhase2RenderPass.h"

#include "VisibilityBufferPhase1Pipeline.h"
#include "VisibilityBufferPhase2Pipeline.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorRenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(instance, surface);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer(0);
		m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
		m_mainCamera->SetMainCamera(true);
		m_renderDescriptors->GetSceneInfoDescriptorSet()->AddCamera(m_mainCamera);
	}

	void EditorRenderSubsystem::Update(const UpdateData& updateData)
	{
		__super::Update(updateData);
	}

	void EditorRenderSubsystem::EditorRender()
	{

	}

	void EditorRenderSubsystem::ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY)
	{
		// todo. std::mutexを追加してスレッドセーフにする

		if (m_vbDispatchInfoCount <= 0 || !m_mainCamera) return;

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

		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
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
				Logger::Debug("Selected object: " + std::to_string(vbDispatchInfo->objectID));
			}
		}

		// 後片付け
		vmaDestroyBuffer(m_device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
		m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
		vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device, m_renderDescriptors);
		m_resources->Initialize(m_swapchain->GetScreenSize());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		__super::initializeRenderPass();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		__super::initializeRenderPipeline();
	}
}