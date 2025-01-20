// Copyright (c) 2025 kong9812
// For include VMA(VulkanMemoryAllocator)
#pragma once
#include <vk_mem_alloc.h>
#include <string_view>

namespace vmaTools
{
    constexpr std::string_view VkSystemAllocationScopeToString(VkSystemAllocationScope systemAllocationScope)
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

    constexpr std::string_view VkInternalAllocationTypeToString(VkInternalAllocationType internalAllocationType)
    {
        switch (internalAllocationType) {
        case VkInternalAllocationType::VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE:  return "VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE";
        case VkInternalAllocationType::VK_INTERNAL_ALLOCATION_TYPE_MAX_ENUM:    return "VK_INTERNAL_ALLOCATION_TYPE_MAX_ENUM";
        default:                                                                return "UNKNOWN";
        }
    }
}