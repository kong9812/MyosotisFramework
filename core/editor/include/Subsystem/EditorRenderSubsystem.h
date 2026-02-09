// Copyright (c) 2025 kong9812
#pragma once
#include <functional>
#include "RenderSubsystem.h"
#include "ThreadSaveValue.h"
#include "MemoryUsage.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
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
		class GridRenderPass;
		TYPEDEF_SHARED_PTR_FWD(GridRenderPass);

		class GizmoPipeline;
		TYPEDEF_SHARED_PTR_FWD(GizmoPipeline);
		class GridPipeline;
		TYPEDEF_SHARED_PTR_FWD(GridPipeline);
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
			m_gizmo(nullptr),
			m_gizmoRenderPass(nullptr),
			m_gridRenderPass(nullptr),
			m_gizmoPipeline(nullptr),
			m_gridPipeline(nullptr),
			m_objectSelectCommandPool(VK_NULL_HANDLE),
			m_objectMovedCallback(nullptr),
			m_objectSelectedCallback(nullptr),
			m_memoryUsage() {
		}
		~EditorRenderSubsystem();

		void Initialize(const VkInstance& instance, const VkSurfaceKHR& surface) override;
		void Update(const UpdateData& updateData) override;
		void EditorRender();
		void ObjectSelect(const int32_t& cursorPosX, const int32_t& cursorPosY);
		const MObject_ptr* GetMObjectPtr(const MObject* raw);

		void ResetCamera();
		void SetCameraFocusObject(const MObject_ptr& object);
		void SelectObject(const MObject_ptr& object);

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

	private:
		Gizmo_ptr m_gizmo;

		GizmoRenderPass_ptr m_gizmoRenderPass;
		GridRenderPass_ptr m_gridRenderPass;

		GizmoPipeline_ptr m_gizmoPipeline;
		GridPipeline_ptr m_gridPipeline;


	private:
		VkCommandPool m_objectSelectCommandPool;

	private:
		std::function<void(void)> m_objectMovedCallback;
		std::function<void(MObject_ptr)> m_objectSelectedCallback;

	public:
		void SetObjectMovedCallback(const std::function<void(void)>& callback) { m_objectMovedCallback = callback; }
		void SetObjectSelectedCallback(const std::function<void(MObject_ptr)>& callback) { m_objectSelectedCallback = callback; }

	public:
		const MemoryUsage& GetMemoryUsage() const { return m_memoryUsage; }

	private:
		MemoryUsage m_memoryUsage;

	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderSubsystem);
}