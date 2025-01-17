#pragma once
#include "libs/glfw.h"
#include "applicationInterface.h"

#ifdef DLL_EXPORTS
#define DLL_CLASS __declspec(dllexport)
#else
#define DLL_CLASS __declspec(dllimport)
#endif

class DLL_CLASS Application : public IApplication
{
public:
	Application(bool allowHotReload);
	~Application();

	int Run();

private:
	GLFWwindow* m_glfwWindow;
	bool m_allowHotReload;
};

extern "C" DLL_CLASS IApplication* GetInstance();