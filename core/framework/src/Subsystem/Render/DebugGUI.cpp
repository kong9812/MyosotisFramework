// Copyright (c) 2025 kong9812
#include "DebugGUI.h"
#include <sstream>

#include "AppInfo.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

namespace MyosotisFW::System::Render
{
	DebugGUI::DebugGUI(
		GLFWwindow& glfwWindow,
		VkInstance& instance,
		RenderDevice_ptr renderDevice,
		VkQueue& queue,
		VkRenderPass& renderPass,
		RenderSwapchain_ptr renderSwapchain,
		VkPipelineCache& pipelineCache)
	{
		m_deltaTime = {};
		m_descriptorPool = VK_NULL_HANDLE;

		m_device = renderDevice;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // キーボード操作を有効化
		io.Fonts->AddFontFromFileTTF((std::string(AppInfo::g_fontFolder) + AppInfo::g_imguiFontFileName).c_str(), AppInfo::g_imguiFontSize);

		ImGui_ImplVulkan_InitInfo initinfo{};
		initinfo.Instance = instance;
		initinfo.PhysicalDevice = m_device->GetPhysicalDevice();
		initinfo.Device = *m_device;
		initinfo.QueueFamily = m_device->GetGraphicsFamilyIndex();
		initinfo.Queue = queue;
		initinfo.DescriptorPoolSize = AppInfo::g_imguiDescriptorPoolSize;       // Set to create internal descriptor pool instead of using DescriptorPool
		initinfo.RenderPass = renderPass;                                       // Ignored if using dynamic rendering
		initinfo.MinImageCount = renderSwapchain->GetMinImageCount();           // >= 2
		initinfo.ImageCount = renderSwapchain->GetImageCount();                 // >= MinImageCount
		initinfo.MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;    // 0 defaults to VK_SAMPLE_COUNT_1_BIT
		initinfo.PipelineCache = pipelineCache;
		initinfo.UseDynamicRendering = false;
		initinfo.Allocator = m_device->GetAllocationCallbacks();
		//initinfo.CheckVkResultFn =  // todo.

		ImGui_ImplVulkan_Init(&initinfo);
		ImGui_ImplGlfw_InitForVulkan(&glfwWindow, true);
	}

	DebugGUI::~DebugGUI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void DebugGUI::Update(UpdateData updateData)
	{
		m_deltaTime = updateData.deltaTime;
	}

	void DebugGUI::BeginDebugCommandBuffer()
	{
#ifndef RELEASE
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
#endif // !RELEASE
	}

	void DebugGUI::BindDebugGUIElement() const
	{
#ifndef RELEASE
		ImGui::SetNextWindowPos({ 0.0f, 0.0f });
		ImGui::Begin("solution configuration",
			(bool*)true,
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
#ifdef DEBUG
		ImGui::Text("Model: Debug");
#elif FWDLL
		ImGui::Text("Model: DLL\nF5: Hot reload");
#endif
		ImGui::Text("FPS: %.2f(%.2fms)", (1.0f / m_deltaTime), m_deltaTime * 1000.0f);
		ImGui::End();
#endif // !RELEASE
	}

	void DebugGUI::EndDebugCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
#ifndef RELEASE
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
#endif // !RELEASE
	}
}