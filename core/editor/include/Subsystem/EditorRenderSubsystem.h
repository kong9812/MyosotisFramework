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

		class Gizmo;
		TYPEDEF_SHARED_PTR_FWD(Gizmo);

		class GizmoRenderPass;
		TYPEDEF_SHARED_PTR_FWD(GizmoRenderPass);

		class GizmoPipeline;
		TYPEDEF_SHARED_PTR_FWD(GizmoPipeline);
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
			m_selectedObject(nullptr),
			m_gizmoRenderPass(nullptr),
			m_gizmoPipeline(nullptr),
			m_gizmo(nullptr),
			m_objectSelectCommandPool(VK_NULL_HANDLE) {
		}
		~EditorRenderSubsystem();

		void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface) override;
		void Update(const UpdateData& updateData) override;
		void EditorRender();
		void ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY);

	private:
		void render(const uint32_t currentFrameIndex) override;

	private:
		void initializeGizmo();
		void initializeRenderResources() override;
		void initializeRenderPass() override;
		void initializeRenderPipeline() override;
		void initializeObjectSelectCommandPool();

		void resizeRenderPass() override;
		void resizeRenderPipeline() override;

	private:
		EditorGUI_ptr m_editorGUI;
		EditorRenderPass_ptr m_editorRenderPass;
		ThreadSaveValue<ComponentBase_ptr> m_selectedObject;

	private:
		GizmoRenderPass_ptr m_gizmoRenderPass;
		GizmoPipeline_ptr m_gizmoPipeline;
		Gizmo_ptr m_gizmo;

	private:
		VkCommandPool m_objectSelectCommandPool;

	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderSubsystem);
}