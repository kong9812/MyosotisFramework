// Copyright (c) 2025 kong9812
#include "EditorManager.h"

#include "EditorRenderSubsystem.h"

namespace MyosotisFW::System
{
	void EditorManager::Initialize(GLFWwindow* window)
	{
		__super::Initialize(window);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	void EditorManager::Update()
	{
		__super::Update();
	}

	void EditorManager::Render()
	{
		m_renderSubsystem->Render();
		//std::dynamic_pointer_cast<Render::EditorRenderSubsystem>(m_renderSubsystem)->EditorRender();
	}

	void EditorManager::initializeRenderSubsystem(GLFWwindow* window)
	{
		m_renderSubsystem = Render::CreateEditorRenderSubsystemPointer();
		m_renderSubsystem->Initialize(m_instance, m_surface);
		//ImGui_ImplGlfw_InitForVulkan(window, true);
	}

	void EditorManager::initializeGameDirector()
	{
		__super::initializeGameDirector();
	}
}