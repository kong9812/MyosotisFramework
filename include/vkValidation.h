#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include "logger.h"

namespace
{
    /// <summary>
    /// vulkanŒ‹‰Ê -> string
    /// </summary>
    /// <param name="result">vulkanŒ‹‰Ê</param>
    /// <returns>•¶Žš—ñ</returns>
    inline constexpr char* vkResultToString(VkResult result) 
    {
        switch (result) {
        case VkResult::VK_SUCCESS: return "VK_SUCCESS";
        case VkResult::VK_NOT_READY: return "VK_NOT_READY";
        case VkResult::VK_TIMEOUT: return "VK_TIMEOUT";
        case VkResult::VK_EVENT_SET: return "VK_EVENT_SET";
        case VkResult::VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VkResult::VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VkResult::VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VkResult::VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VkResult::VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VkResult::VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VkResult::VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VkResult::VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VkResult::VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VkResult::VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VkResult::VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        default: return "Unknown VkResult code";
        }
    }
}

#define VK_VALIDATION(result)                                                                   \
{                                                                                               \
    VkResult validationResult = static_cast<VkResult>(result);                                  \
    bool validationCondition = (validationResult == VkResult::VK_SUCCESS);                      \
    if (!validationCondition) {                                                                 \
                                                                                                \
                                                                                                \
            std::string message = "\nVulkan Error " + std::string(vkResultToString(result));    \
            ASSERT(validationCondition, message);                                               \
    }                                                                                           \
}