#define DLL_EXPORTS

#include "application.h"

#include <crtdbg.h>
#include <memory>
#include "systemManager.h"
#include "logger.h"
#include "appInfo.h"

Application::Application(bool allowHotReload)
{
#ifdef DEBUG
	Logger::ClearLog();
#endif
    // メモリリークチェッカ
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // GLFW初期化
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Window作成
    m_glfwWindow = glfwCreateWindow(
        MyosotisFW::AppInfo::g_windowWidth,
        MyosotisFW::AppInfo::g_windowHeight,
        MyosotisFW::AppInfo::g_applicationName,
        nullptr, nullptr);

    m_allowHotReload = allowHotReload;
}

Application::~Application()
{
    glfwTerminate();
}

int Application::Run()
{
    // System Manager 初期化
    MyosotisFW::System::SystemManager_ptr systemManager = MyosotisFW::System::CreateSystemManagerPointer(m_glfwWindow);
    while (glfwWindowShouldClose(m_glfwWindow) == GLFW_FALSE)
    {
        glfwPollEvents();

        // マウスボタンの反応がないため、こちらで追加 (一時対応)
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[0] = (glfwGetMouseButton(m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);   // 左クリック
        io.MouseDown[1] = (glfwGetMouseButton(m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);  // 右クリック
        io.MouseDown[2] = (glfwGetMouseButton(m_glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS); // 中クリック

        systemManager->Update();
        systemManager->Render();

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
        }

        if ((glfwGetKey(m_glfwWindow, GLFW_KEY_F5) == GLFW_PRESS) && m_allowHotReload) {
            glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
            return 2;
        }
    }

	return 0;
}

extern "C" __declspec(dllexport) IApplication* GetInstance()
{
    return new Application(true);
}