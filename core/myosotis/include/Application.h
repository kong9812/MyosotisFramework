// Copyright (c) 2025 kong9812
#pragma once
#include "iglfw.h"
#include "ApplicationInterface.h"

class Application : public IApplication
{
public:
	Application() : m_glfwWindow(nullptr), m_allowHotReload(false) {};
	~Application();

	void Initialize(const bool& allowHotReload) override;
	int Run() override;

private:
	GLFWwindow* m_glfwWindow;
	bool m_allowHotReload;
};

IApplication* GetInstance();