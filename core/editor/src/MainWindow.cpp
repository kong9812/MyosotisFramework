// Copyright (c) 2025 kong9812
#include "MainWindow.h"
#include "Appinfo.h"

namespace MyosotisFW::System::Editor
{
	MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) :
		QMainWindow(parent, flags),
		m_contentBrowser(new ContentBrowserDockWidget(this)),
		m_logger(new LoggerDockWidget(this))
	{
		setWindowIcon(QIcon(AppInfo::g_applicationIcon));
		setAccessibleName(AppInfo::g_applicationName);
		resize(QSize(AppInfo::g_windowWidth, AppInfo::g_windowHeight));

		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_contentBrowser);
		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_logger);
		resizeDocks({ m_contentBrowser, m_logger }, { 300, 150 }, Qt::Orientation::Horizontal);
		resizeDocks({ m_contentBrowser, m_logger }, { 250, 250 }, Qt::Orientation::Vertical);
	}

	void MainWindow::closeEvent(QCloseEvent* event)
	{
		__super::closeEvent(event);
	}

	void MainWindow::paintEvent(QPaintEvent* event)
	{
		__super::paintEvent(event);
	}

	void MainWindow::closeWindow()
	{
		close();
	}

	void MainWindow::openFile(std::string filePath)
	{

	}

	void MainWindow::resetLayout()
	{

	}
}