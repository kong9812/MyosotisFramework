// Copyright (c) 2025 kong9812
#include "EditorRenderSubsystem.h"

#include "MObject.h"

#include "EditorRenderResources.h"
#include "RenderSwapchain.h"
#include "RenderDevice.h"
#include "RenderQueue.h"

#include "EditorGUI.h"
#include "EditorCamera.h"
#include "StaticMesh.h"

#include "VisibilityBufferPhase1RenderPass.h"
#include "VisibilityBufferPhase2RenderPass.h"

#include "VisibilityBufferPhase1Pipeline.h"
#include "VisibilityBufferPhase2Pipeline.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorRenderSubsystem::Initialize(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(instance, surface);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer(0);
		m_mainCamera->UpdateScreenSize(m_swapchain->GetScreenSizeF());
	}

	void EditorRenderSubsystem::Update(const UpdateData& updateData)
	{
		__super::Update(updateData);
	}

	void EditorRenderSubsystem::EditorRender()
	{

	}

	void EditorRenderSubsystem::ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY)
	{

	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device, m_renderDescriptors);
		m_resources->Initialize(m_swapchain->GetScreenSize());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		__super::initializeRenderPass();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		__super::initializeRenderPipeline();
	}
}