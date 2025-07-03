// Copyright (c) 2025 kong9812
#include "EditorGUI.h"
#include <sstream>

#include "AppInfo.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

namespace MyosotisFW::System::Render
{
	EditorGUI::EditorGUI(
		const VkInstance& instance,
		const RenderDevice_ptr renderDevice,
		const VkRenderPass& renderPass,
		const RenderSwapchain_ptr renderSwapchain)
	{
		m_descriptorPool = VK_NULL_HANDLE;

		m_device = renderDevice;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // キーボード操作を有効化
		io.Fonts->AddFontFromFileTTF((std::string(AppInfo::g_fontFolder) + AppInfo::g_imguiFontFileName).c_str(), AppInfo::g_imguiFontSize);
		RenderQueue_ptr graphicsQueue = m_device->GetGraphicsQueue();
		ImGui_ImplVulkan_InitInfo initinfo{};
		initinfo.Instance = instance;
		initinfo.PhysicalDevice = m_device->GetPhysicalDevice();
		initinfo.Device = *m_device;
		initinfo.QueueFamily = graphicsQueue->GetQueueFamilyIndex();
		initinfo.Queue = graphicsQueue->GetQueue();
		initinfo.DescriptorPoolSize = AppInfo::g_imguiDescriptorPoolSize;       // Set to create internal descriptor pool instead of using DescriptorPool
		initinfo.RenderPass = renderPass;                                       // Ignored if using dynamic rendering
		initinfo.MinImageCount = renderSwapchain->GetMinImageCount();           // >= 2
		initinfo.ImageCount = renderSwapchain->GetImageCount();                 // >= MinImageCount
		initinfo.MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;    // 0 defaults to VK_SAMPLE_COUNT_1_BIT
		initinfo.PipelineCache = VK_NULL_HANDLE;
		initinfo.UseDynamicRendering = false;
		initinfo.Allocator = m_device->GetAllocationCallbacks();
		//initinfo.CheckVkResultFn =  // todo.

		ImGui_ImplVulkan_Init(&initinfo);
	}

	void EditorGUI::Update(const UpdateData& updateData)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({ 0.0f, 0.0f });
		ImGui::Begin("MainEditorWindow",
			(bool*)true,
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
		ImGui::End();
	}

	EditorGUI::~EditorGUI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		//ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}