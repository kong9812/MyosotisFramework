// Copyright (c) 2025 kong9812
// For include VMA(VulkanMemoryAllocator)
#pragma once
#include <vk_mem_alloc.h>
#include <string_view>
#include "VK_Validation.h"

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

	template <typename T>
	inline void ShaderBufferObjectAllocate(
		const VkDevice& device,
		const VmaAllocator& allocator,
		const T& data,
		const VkBufferUsageFlagBits& usage,
		VkBuffer& pBuffer,
		VmaAllocation& pAllocation,
		VmaAllocationInfo& pAllocationInfo,
		VkDescriptorBufferInfo& descriptor)
	{
		{// m_staticMeshUniformBufferObject
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(data), usage);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;					// CPUで更新可能
			allocationCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;	// 永続マッピング
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &pBuffer, &pAllocation, &pAllocationInfo));
			descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(pBuffer);
		}
	}

	template <typename T>
	inline void ShaderBufferObjectAllocate(
		const VkDevice& device,
		const VmaAllocator& allocator,
		const T& data,
		const uint32_t& size,
		const VkBufferUsageFlagBits& usage,
		VkBuffer& pBuffer,
		VmaAllocation& pAllocation,
		VmaAllocationInfo& pAllocationInfo,
		VkDescriptorBufferInfo& descriptor)
	{
		{// m_staticMeshUniformBufferObject
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(data), usage);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;					// CPUで更新可能
			allocationCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;	// 永続マッピング
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &pBuffer, &pAllocation, &pAllocationInfo));
			descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(pBuffer);
		}
	}
}