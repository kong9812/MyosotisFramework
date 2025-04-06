// Copyright (c) 2025 kong9812
#include "EditorRenderSubsystem.h"
#include "EditorRenderResources.h"

#include "EditorFinalCompositionRenderPass.h"
#include "EditorFinalCompositionRenderPipeline.h"
#include "EditorCamera.h"

#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorRenderSubsystem::Initialize(GLFWwindow& glfwWindow, const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		__super::Initialize(glfwWindow, instance, surface);

		// EditorGUIの初期化
		m_editorGUI = CreateEditorGUIPointer(const_cast<GLFWwindow&>(glfwWindow), instance, m_device, m_graphicsQueue, m_editorRenderPass->GetRenderPass(), m_swapchain);

		// EditorCameraの初期化
		m_mainCamera = Camera::CreateEditorCameraPointer();
	}

	void EditorRenderSubsystem::Update(const UpdateData& updateData)
	{
		m_editorGUI->NewFrame();
		ImGui::ShowDemoWindow();

		ImGui::Begin("MainEditorWindow");
#ifdef DEBUG
		ImGui::Text("Model: Debug");
#elif FWDLL
		ImGui::Text("Model: DLL\nF5: Hot reload");
#elif RELEASE
		ImGui::Text("Model: Release");
#endif
		ImGui::Text("FPS: %.2f(%.2fms)", (1.0f / updateData.deltaTime), updateData.deltaTime * 1000.0f);
		ImGui::End();

		__super::Update(updateData);
	}

	void EditorRenderSubsystem::EditorRender()
	{
		VkCommandBuffer currentCommandBuffer = m_renderCommandBuffers[m_currentBufferIndex];
		m_editorRenderPass->BeginRender(currentCommandBuffer, m_currentBufferIndex);
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currentCommandBuffer);
		m_editorRenderPass->EndRender(currentCommandBuffer);
	}

	void EditorRenderSubsystem::initializeRenderResources()
	{
		m_resources = CreateEditorRenderResourcesPointer(m_device);
		m_resources->Initialize(m_swapchain->GetWidth(), m_swapchain->GetHeight());
	}

	void EditorRenderSubsystem::initializeRenderPass()
	{
		m_shadowMapRenderPass = CreateShadowMapRenderPassPointer(m_device, m_resources, AppInfo::g_shadowMapSize, AppInfo::g_shadowMapSize);
		m_shadowMapRenderPass->Initialize();

		m_mainRenderPass = CreateMainRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_mainRenderPass->Initialize();

		m_finalCompositionRenderPass = CreateEditorFinalCompositionRenderPassPointer(m_device, std::dynamic_pointer_cast<EditorRenderResources>(m_resources), m_swapchain);
		m_finalCompositionRenderPass->Initialize();

		m_editorRenderPass = CreateEditorRenderPassPointer(m_device, m_resources, m_swapchain->GetWidth(), m_swapchain->GetHeight());
		m_editorRenderPass->Initialize();
	}

	void EditorRenderSubsystem::initializeRenderPipeline()
	{
		m_shadowMapRenderPipeline = CreateShadowMapRenderPipelinePointer(m_device);
		m_shadowMapRenderPipeline->Initialize(m_resources, m_shadowMapRenderPass->GetRenderPass());

		m_skyboxRenderPipeline = CreateSkyboxRenderPipelinePointer(m_device);
		m_skyboxRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_deferredRenderPipeline = CreateDeferredRenderPipelinePointer(m_device);
		m_deferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_lightingRenderPipeline = CreateLightingRenderPipelinePointer(m_device);
		m_lightingRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_compositionRenderPipeline = CreateCompositionRenderPipelinePointer(m_device);
		m_compositionRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_interiorObjectDeferredRenderPipeline = CreateInteriorObjectDeferredRenderPipelinePointer(m_device);
		m_interiorObjectDeferredRenderPipeline->Initialize(m_resources, m_mainRenderPass->GetRenderPass());

		m_finalCompositionRenderPipeline = CreateEditorFinalCompositionRenderPipelinePointer(m_device);
		m_finalCompositionRenderPipeline->Initialize(m_resources, m_finalCompositionRenderPass->GetRenderPass());
		m_lightingRenderPipeline->UpdateDirectionalLightInfo(m_shadowMapRenderPipeline->GetDirectionalLightInfo());

		m_lightingRenderPipeline->CreateShaderObject(m_shadowMapRenderPipeline->GetShadowMapDescriptorImageInfo());
		m_compositionRenderPipeline->CreateShaderObject();
		m_finalCompositionRenderPipeline->CreateShaderObject();
	}
}