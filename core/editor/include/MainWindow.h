// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ContentBrowserDockWidget.h"
#include "LoggerDockWidget.h"

namespace MyosotisFW::System::Editor
{
	class MainWindow : public QMainWindow
	{
	public:
		MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	private:
		ContentBrowserDockWidget* m_contentBrowser;
		LoggerDockWidget* m_logger;

		void closeEvent(QCloseEvent* event) override;
		void paintEvent(QPaintEvent* event) override;

	private:
		void closeWindow();
		void openFile(std::string filePath);

	public slots:
		void resetLayout();
	};
}