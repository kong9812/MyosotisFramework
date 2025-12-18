// For include VMA(VulkanMemoryAllocator)
#pragma once
#include <vk_mem_alloc.h>
#include <string_view>
#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "Buffer.h"

namespace vmaTools
{
	constexpr std::string_view VkSystemAllocationScopeToString(const VkSystemAllocationScope& systemAllocationScope)
	{
		switch (systemAllocationScope) {
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:   return "VK_SYSTEM_ALLOCATION_SCOPE_COMMAND";
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:    return "VK_SYSTEM_ALLOCATION_SCOPE_OBJECT";
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_CACHE:     return "VK_SYSTEM_ALLOCATION_SCOPE_CACHE";
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:    return "VK_SYSTEM_ALLOCATION_SCOPE_DEVICE";
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:  return "VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE";
		case VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM:  return "VK_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM";
		default:                                                            return "UNKNOWN";
		}
	}

	constexpr std::string_view VkInternalAllocationTypeToString(const VkInternalAllocationType& internalAllocationType)
	{
		switch (internalAllocationType) {
		case VkInternalAllocationType::VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE:  return "VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE";
		case VkInternalAllocationType::VK_INTERNAL_ALLOCATION_TYPE_MAX_ENUM:    return "VK_INTERNAL_ALLOCATION_TYPE_MAX_ENUM";
		default:                                                                return "UNKNOWN";
		}
	}

	inline MyosotisFW::Buffer CreateBuffer(
		const VmaAllocator& allocator,
		const VkDeviceSize size,
		const VkBufferUsageFlags usage,
		const VmaMemoryUsage vmaUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN,
		const VmaAllocationCreateFlags vmaFlags = 0)
	{
		MyosotisFW::Buffer buffer{};
		VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(size, usage);
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = vmaUsage;
		allocationCreateInfo.flags = vmaFlags;
		VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer.buffer, &buffer.allocation, &buffer.allocationInfo));
		buffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(buffer.buffer);
		buffer.localSize = size;
		VkMemoryPropertyFlags memoryPropertyFlags{};
		vmaGetAllocationMemoryProperties(allocator, buffer.allocation, &memoryPropertyFlags);
		buffer.needFlush =
			(memoryPropertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
			!(memoryPropertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		return buffer;
	}

	// SSBO作成用
	inline MyosotisFW::Buffer CreateShaderStorageBuffer(const VmaAllocator& allocator, const VkDeviceSize size, const VkBufferUsageFlags extraUsage = 0)
	{
		return CreateBuffer(allocator, size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraUsage,
			VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
			VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);
	}

	// AccelerationStructureBuffer (AS Buffer) 作成用
	inline MyosotisFW::Buffer CreateASBuffer(const VmaAllocator& allocator, const VkDeviceSize size)
	{
		return CreateBuffer(allocator, size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
	}

	// ScratchBuffer 作成用
	inline MyosotisFW::Buffer CreateScratchBuffer(const VmaAllocator& allocator, const VkDeviceSize size)
	{
		return CreateBuffer(allocator, size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
	}

	// AccelerationStructureInstanceBuffer (ASInstance Buffer) 作成用
	inline MyosotisFW::Buffer CreateASInstanceBuffer(const VmaAllocator& allocator, const VkDeviceSize size)
	{
		return CreateBuffer(allocator, size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
			VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);
	}

	inline void MemcpyBufferData(const VmaAllocator& allocator, MyosotisFW::Buffer& buffer, const void* src, VkDeviceSize size, VkDeviceSize dstOffset = 0)
	{
		if ((!buffer.allocationInfo.pMappedData) || (!src) || (size == 0)) return;

		uint8_t* dst = static_cast<uint8_t*>(buffer.allocationInfo.pMappedData) + dstOffset;
		memcpy(dst, src, static_cast<size_t>(size));
		if (buffer.needFlush)
		{
			vmaFlushAllocation(allocator, buffer.allocation, dstOffset, size);
		}
	}
}