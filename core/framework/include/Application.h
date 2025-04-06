// Copyright (c) 2025 kong9812
#pragma once
#ifndef EDITOR
#include "iglfw.h"
#include "ApplicationInterface.h"

#ifdef DLL_EXPORTS
#define DLL_CLASS __declspec(dllexport)
#else
#define DLL_CLASS __declspec(dllimport)
#endif

class DLL_CLASS Application : public IApplication
{
public:
	Application(const bool& allowHotReload);
	~Application();

	int Run();

private:
	GLFWwindow* m_glfwWindow;
	bool m_allowHotReload;
};

extern "C" DLL_CLASS IApplication* GetInstance();
#endif