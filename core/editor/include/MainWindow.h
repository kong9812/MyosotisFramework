// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "VulkanWindow.h"
#include "ContentBrowserDockWidget.h"
#include "LoggerDockWidget.h"
#include "OverviewDockWidget.h"
#include "PropertyViewerDockWidget.h"

namespace MyosotisFW::System::Editor
{
	class MainWindow : public QMainWindow
	{
	public:
		MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	private:
		VulkanWindow* m_vulkanWindow;
		ContentBrowserDockWidget* m_contentBrowser;
		LoggerDockWidget* m_logger;
		OverviewDockWidget* m_overview;
		PropertyViewerDockWidget* m_propertyViewer;
		QStatusBar* m_statusBar;
		QToolBar* m_toolBar;

		void closeEvent(QCloseEvent* event) override;
		void paintEvent(QPaintEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

		void connectDockWidgetsSignals();

	private:
		void closeWindow();
		void openFile(std::string filePath);

	public slots:
		void resetLayout();
	};
}