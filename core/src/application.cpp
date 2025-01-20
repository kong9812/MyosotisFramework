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

    // 画面中央
    int monitorCount{};
    GLFWmonitor* monitor = glfwGetMonitors(&monitorCount)[MyosotisFW::AppInfo::g_montorIndex];
    const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(m_glfwWindow,
        static_cast<int>((vidMode->width - MyosotisFW::AppInfo::g_windowWidth) / 2),
        static_cast<int>((vidMode->height - MyosotisFW::AppInfo::g_windowHeight) / 2));
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