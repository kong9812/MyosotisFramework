// Copyright (c) 2025 kong9812
#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

namespace MyosotisFW::AppInfo
{
    // アプリケーション情報
    constexpr uint32_t g_windowWidth = 640;
    constexpr uint32_t g_windowHeight = 480;
    constexpr char* g_applicationName = "MyosotisFW";
    constexpr char* g_engineName = "MyosotisFW";
    constexpr uint32_t g_apiVersion = VK_API_VERSION_1_3;
    constexpr uint32_t g_engineVersion = VK_MAKE_VERSION(0, 0, 1);

    // 指定VkExtensionProperties
    const std::vector<const char*> g_vkInstanceExtensionProperties = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };
    // 指定VkExtensionProperties
    const std::vector<const char*> g_vkDeviceExtensionProperties = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    // 指定Layer
    const std::vector<const char*> g_layer = {
        "VK_LAYER_KHRONOS_validation",
    };
    // 指定物理デバイスのIndex
    constexpr uint32_t g_physicalIndex = 0;
    // 指定画像フォーマット
    constexpr VkSurfaceFormatKHR g_surfaceFormat = { VkFormat::VK_FORMAT_R8G8B8A8_UNORM, VkColorSpaceKHR ::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    // 指定プレゼントモード
    constexpr VkPresentModeKHR g_presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
    // 指定DepthFormat
    constexpr VkFormat g_depthFormat = VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
    // 指定スワップチェーン画像数
    constexpr uint32_t g_minImageCount = 3;

    // 背景色
    constexpr VkClearValue g_colorClearValues = { 0.0f, 0.2f, 0.2f, 1.0f };
    // depth
    constexpr VkClearValue g_depthClearValues = { 1.0f, 0 };

    // [デフォルト]カメラ位置
    constexpr glm::vec3 g_cameraPos = glm::vec3(0.0f, 2.0f, -5.0f);
    // [デフォルト]カメラ視点
    constexpr glm::vec3 g_cameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
    // [デフォルト]カメラ　前ベクトル
    constexpr glm::vec3 g_cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    // [デフォルト]カメラ　上ベクトル
    constexpr glm::vec3 g_cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
    // [デフォルト]カメラ　右ベクトル
    constexpr glm::vec3 g_cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    // [デフォルト]カメラ FOV
    constexpr float g_cameraFov = 60.0f;
    // [デフォルト]カメラ Far
    constexpr float g_cameraFar = 1000.0f;
    // [デフォルト]カメラ Near
    constexpr float g_cameraNear = 0.1f;

}
