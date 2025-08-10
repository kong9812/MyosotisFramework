// Copyright (c) 2025 kong9812
#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "iglm.h"

#ifdef _WIN32
#include <wtypes.h>
#include <vulkan/vulkan_win32.h>
#elif __APPLE__
#include <vulkan/vulkan_macos.h>
#endif

namespace MyosotisFW::AppInfo
{
	// アプリケーション情報
	constexpr uint32_t g_windowWidth = 1440;
	constexpr uint32_t g_windowHeight = 810;
	constexpr uint32_t g_montorIndex = 0;
	constexpr const char* g_applicationName = "MyosotisFW";
	constexpr const char* g_editorName = "MyosotisEditor";
	constexpr const char* g_engineName = "MyosotisFW";
	constexpr uint32_t g_apiVersion = VK_API_VERSION_1_3;
	constexpr uint32_t g_engineVersion = VK_MAKE_VERSION(0, 0, 1);
	constexpr const char* g_applicationIcon = ".\\resources\\appInfo\\MyosotisFW.png";

	// 指定VkExtensionProperties
	const std::vector<const char*> g_vkInstanceExtensionProperties = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif __APPLE__
		VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
	};
	// 指定VkExtensionProperties
	const std::vector<const char*> g_vkDeviceExtensionProperties = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_MESH_SHADER_EXTENSION_NAME,
		VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
	};
	// 指定Layer
	const std::vector<const char*> g_layer = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
	};
	// 指定物理デバイスのIndex
	constexpr uint32_t g_physicalIndex = 0;
	// 指定画像フォーマット
	constexpr VkSurfaceFormatKHR g_surfaceFormat = { VkFormat::VK_FORMAT_R8G8B8A8_SRGB, VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; // 表示用に最適化されており、多くのディスプレイがsRGBカラースペースでカラーを直接表示できる
	// 指定プレゼントモード
	constexpr VkPresentModeKHR g_presentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;  // FIFO: vsync
	// 指定DepthFormat
	constexpr VkFormat g_depthFormat = VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
	// 指定Primary DepthFormat
	constexpr VkFormat g_primaryDepthFormat = VkFormat::VK_FORMAT_D32_SFLOAT;
	// 指定Hi-Z DepthFormat
	constexpr VkFormat g_hiZDepthFormat = VkFormat::VK_FORMAT_D32_SFLOAT;
	// 指定画像フォーマット ([deferred] position)
	constexpr VkFormat g_deferredPositionFormat = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT; // HDR&精密な色管理が可能
	// 指定画像フォーマット ([deferred] normal)
	constexpr VkFormat g_deferredNormalFormat = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT; // HDR&精密な色管理が可能
	// 指定画像フォーマット
	constexpr VkFormat g_colorFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;  // 0.0～1.0
	// 指定画像フォーマット (shadow map)
	constexpr VkFormat g_shadowMapFormat = VkFormat::VK_FORMAT_D32_SFLOAT;
	// 指定画像フォーマット (id map)
	constexpr VkFormat g_idMapFormat = VkFormat::VK_FORMAT_R32_UINT;
	// 指定スワップチェーン画像数
	constexpr uint32_t g_minImageCount = 3;
	// 指定Hi-Z Mipmapレベル数
	constexpr uint32_t g_hiZMipLevels = 5;

	// シャドウマップサイズ
	constexpr uint32_t g_shadowMapSize = 2048;

	// 背景色
	constexpr VkClearValue g_colorClearValues = { 0.0f, 0.0f, 0.0f, 0.0f };
	// depth/stencil
	constexpr VkClearValue g_depthStencilClearValues = { 1.0f, 0.0f };
	// depth
	constexpr VkClearValue g_depthClearValues = { 1.0f };

	// [デフォルト]カメラ位置
	constexpr glm::vec3 g_cameraPos = glm::vec3(0.0f, 0.0f, -20.0f);
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
	constexpr float g_cameraFar = 256.0f;
	// [デフォルト]カメラ Near
	constexpr float g_cameraNear = 1.0f;

	// [フォルダ]リソース
	constexpr const char* g_resourcesFolder = ".\\resources\\";
	// [フォルダ]シェーダー
	constexpr const char* g_shaderFolder = ".\\resources\\shaders\\spv\\";
	// [フォルダ]テクスチャ
	constexpr const char* g_textureFolder = ".\\resources\\texture\\";
	// [フォルダ]フォント
	constexpr const char* g_fontFolder = ".\\resources\\font\\";
	// [フォルダ]モデル
	constexpr const char* g_modelFolder = ".\\resources\\models\\";
	// [フォルダ]ゲームステージ
	constexpr const char* g_gameStageFolder = ".\\resources\\gamestage\\";

	// debug gui (size > IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE)
	constexpr uint32_t g_imguiDescriptorPoolSize = 2;
	constexpr float g_imguiFontSize = 16.0f;
	constexpr const char* g_imguiFontFileName = "msgothic.ttc";

	// [デフォルト]LOD距離
	constexpr float g_defaultLODVeryClose = 10.0f;
	constexpr float g_defaultLODClose = 30.0f;
	constexpr float g_defaultLODFar = 60.0f;

	// 移動
	constexpr float g_moveSpeed = 1.0f;
	constexpr float g_moveBoostSpeed = 10.0f;

	// render pipeline
	constexpr uint32_t g_descriptorCount = 10;

	// max object count
	constexpr uint32_t g_maxObject = 10000;

	// max meshlet count
	constexpr uint32_t g_maxMeshletVertices = 64;
	constexpr uint32_t g_maxMeshletPrimitives = 124;
}