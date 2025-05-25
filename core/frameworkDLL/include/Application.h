// Copyright (c) 2025 kong9812
#pragma once
#include "iglfw.h"
#include "ApplicationInterface.h"

#define DLL_CLASS __declspec(dllexport)

class DLL_CLASS Application : public IApplication
{
public:
	Application() : m_glfwWindow(nullptr), m_allowHotReload(false) {}
	~Application();

	void Initialize(const bool& allowHotReload) override;
	int Run() override;

private:
	GLFWwindow* m_glfwWindow;
	bool m_allowHotReload;
};

extern "C" DLL_CLASS IApplication* GetInstance();