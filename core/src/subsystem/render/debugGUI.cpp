// Copyright (c) 2025 kong9812
#include "debugGUI.h"

#include "appInfo.h"

#include "vkCreateInfo.h"
#include "vkValidation.h"
#include <GLFW/glfw3.h>

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
        m_device = renderDevice;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // キーボード操作を有効化

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
        initinfo.Allocator = nullptr;
        //initinfo.CheckVkResultFn =  // todo.

        ImGui_ImplVulkan_Init(&initinfo);
        ImGui_ImplGlfw_InitForVulkan(&glfwWindow, false);

        m_mainWindow = true;
	}

	DebugGUI::~DebugGUI()
	{
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
	}

    void DebugGUI::BuildCommandBuffer(VkCommandBuffer& commandBuffer)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        ImGui::Begin("Debug GUI");
#ifdef DEBUG
        ImGui::Text("Model: Debug");
#elif FWDLL
        ImGui::Text("Model: DLL");
#elif RELEASE
        ImGui::Text("Model: Release");
#else
        ImGui::Text("Model: Other");
#endif
        float mousePos[2] = { io.MousePos.x, io.MousePos.y };
        ImGui::DragFloat2("MousePos: ", mousePos);
        if (io.MouseDown[1])
        {
            ImGui::Text("Clicked!");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
}