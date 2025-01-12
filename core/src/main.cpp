// Copyright (c) 2025 kong9812
#include <crtdbg.h>
#include <memory>
#include "libs/glfw.h"
#include "systemManager.h"
#include "logger.h"
#include "appInfo.h"

int main()
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
    GLFWwindow* glfwWindow = glfwCreateWindow(
        MyosotisFW::AppInfo::g_windowWidth,
        MyosotisFW::AppInfo::g_windowHeight,
        MyosotisFW::AppInfo::g_applicationName,
        nullptr, nullptr);

    // System Manager 初期化
    MyosotisFW::System::SystemManager_ptr systemManager = MyosotisFW::System::CreateSystemManagerPointer(glfwWindow);
    while (glfwWindowShouldClose(glfwWindow) == GLFW_FALSE)
    {
        glfwPollEvents();

        systemManager->Update();

        if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
        }
    }

    glfwTerminate();
    return 0;
}
