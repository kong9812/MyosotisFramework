// Copyright (c) 2025 kong9812
#include "MainWindow.h"
#include "Appinfo.h"

namespace MyosotisFW::System::Editor
{
	MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) :
		QMainWindow(parent, flags),
		m_vulkanWindow(new VulkanWindow()),
		m_contentBrowser(new ContentBrowserDockWidget(this)),
		m_logger(new LoggerDockWidget(this))
	{
		setWindowIcon(QIcon(AppInfo::g_applicationIcon));
		setAccessibleName(AppInfo::g_applicationName);
		resize(QSize(AppInfo::g_windowWidth, AppInfo::g_windowHeight));

		connect(m_vulkanWindow, &VulkanWindow::closeWindow, this, &MainWindow::closeWindow);
		setCentralWidget(QWidget::createWindowContainer(m_vulkanWindow));

		connect(m_contentBrowser, &ContentBrowserDockWidget::openFile, this, &MainWindow::openFile);

		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_contentBrowser);
		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_logger);
		resizeDocks({ m_contentBrowser, m_logger }, { 300, 150 }, Qt::Orientation::Horizontal);
		resizeDocks({ m_contentBrowser, m_logger }, { 250, 250 }, Qt::Orientation::Vertical);
	}

	void MainWindow::closeEvent(QCloseEvent* event)
	{
		m_vulkanWindow->destroy();
		__super::closeEvent(event);
	}

	void MainWindow::paintEvent(QPaintEvent* event)
	{
		__super::paintEvent(event);
		if (!m_vulkanWindow) return;
	}

	void MainWindow::keyPressEvent(QKeyEvent* event)
	{
		m_vulkanWindow->keyPressEvent(event);
		__super::keyPressEvent(event);
	}

	void MainWindow::keyReleaseEvent(QKeyEvent* event)
	{
		m_vulkanWindow->keyReleaseEvent(event);
		__super::keyReleaseEvent(event);
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
		if (m_contentBrowser->isHidden()) m_contentBrowser->show();
		if (m_logger->isHidden()) m_logger->show();
		m_contentBrowser->setFloating(false);
		m_logger->setFloating(false);
		resizeDocks({ m_contentBrowser, m_logger }, { 300, 150 }, Qt::Orientation::Horizontal);
		resizeDocks({ m_contentBrowser, m_logger }, { 250, 250 }, Qt::Orientation::Vertical);
	}
}