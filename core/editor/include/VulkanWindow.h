// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "EditorRenderSubsystem.h"
#include "GameDirector.h"

namespace MyosotisFW::System::Editor
{
	class VulkanWindow : public QWindow
	{
		Q_OBJECT
	public:
		VulkanWindow(QWindow* parent = nullptr);
		~VulkanWindow();

		void resizeEvent(QResizeEvent* event) override;

		void Initialize();

		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;

	protected:
		bool eventFilter(QObject* watched, QEvent* event) override;
		bool event(QEvent* event) override;
		bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

	private:
		void updateRender();

		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		Render::EditorRenderSubsystem_ptr m_renderSubsystem;
		GameDirector::GameDirector_ptr m_gameDirector;

		bool m_initialized;
		bool m_resizing;
		float m_lastTime;
		QElapsedTimer m_timer;

		// 入力
		std::unordered_map<int, int> m_keyActions;
		std::unordered_map<int, int> m_mouseButtonActions;
		glm::vec2 m_mousePos;

	signals:
		void closeWindow();
	};
}