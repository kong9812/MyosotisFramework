// Copyright (c) 2025 kong9812
#pragma once
#include "RenderSubsystem.h"
#include "ThreadSaveValue.h"

// 前方宣言
namespace MyosotisFW
{
	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);
	namespace System::Render
	{
		class EditorGUI;
		TYPEDEF_SHARED_PTR_FWD(EditorGUI);
		class EditorRenderPass;
		TYPEDEF_SHARED_PTR_FWD(EditorRenderPass);
	}
}

namespace MyosotisFW::System::Render
{
	class EditorRenderSubsystem : public RenderSubsystem
	{
	public:
		EditorRenderSubsystem() : RenderSubsystem(),
			m_editorGUI(nullptr),
			m_editorRenderPass(nullptr),
			m_selectedObject(nullptr) {
		}
		~EditorRenderSubsystem() {}

		void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface) override;
		void Update(const UpdateData& updateData) override;
		void EditorRender();
		void ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY);

	private:
		void initializeRenderResources() override;
		void initializeRenderPass() override;
		void initializeRenderPipeline() override;

	private:
		EditorGUI_ptr m_editorGUI;
		EditorRenderPass_ptr m_editorRenderPass;
		ThreadSaveValue<ComponentBase_ptr> m_selectedObject;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderSubsystem);
}