// Copyright (c) 2025 kong9812
#pragma once
#include "RenderSubsystem.h"
#include "EditorGUI.h"
#include "EditorRenderPass.h"

namespace MyosotisFW::System::Render
{
	class EditorRenderSubsystem : public RenderSubsystem
	{
	public:
		EditorRenderSubsystem() : RenderSubsystem(),
			m_editorGUI(nullptr) {
		}
		~EditorRenderSubsystem() {};

		void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface) override;
		void Update(const UpdateData& updateData) override;
		void EditorRender();

	private:
		void initializeRenderResources() override;
		void initializeRenderPass() override;
		void initializeRenderPipeline() override;
		void resizeRenderPass(const uint32_t& width, const uint32_t& height) override;
		void resizeRenderPipeline() override;

	private:
		EditorGUI_ptr m_editorGUI;
		EditorRenderPass_ptr m_editorRenderPass;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderSubsystem)
}