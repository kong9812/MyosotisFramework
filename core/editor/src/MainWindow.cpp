// Copyright (c) 2025 kong9812
#include "MainWindow.h"
#include "Appinfo.h"
#include "KeyConverter.h"
#include "ThreadPool.h"
#include "istduuid.h"

namespace MyosotisFW::System::Editor
{
	MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) :
		QMainWindow(parent, flags),
		m_vulkanWindow(new VulkanWindow()),
		m_contentBrowser(new ContentBrowserDockWidget(this)),
		m_logger(new LoggerDockWidget(this)),
		m_overview(new OverviewDockWidget(this)),
		m_propertyViewer(new PropertyViewerDockWidget(this))
	{
		// KeyConverterインスタンス作成
		KeyConverter::Instance();

		// ThreadPoolインスタンス作成
		ThreadPool::Instance();

		setWindowIcon(QIcon(AppInfo::g_applicationIcon));
		setAccessibleName(AppInfo::g_applicationName);
		resize(QSize(AppInfo::g_windowWidth, AppInfo::g_windowHeight));

		setCentralWidget(QWidget::createWindowContainer(m_vulkanWindow));

		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_contentBrowser);
		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_logger);
		addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_overview);
		addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_propertyViewer);
		resizeDocks({ m_contentBrowser, m_logger }, { 300, 150 }, Qt::Orientation::Horizontal);
		resizeDocks({ m_contentBrowser, m_logger }, { 250, 250 }, Qt::Orientation::Vertical);
		resizeDocks({ m_overview, m_propertyViewer }, { 250, 250 }, Qt::Orientation::Vertical);
		resizeDocks({ m_overview, m_propertyViewer }, { 300, 300 }, Qt::Orientation::Horizontal);

		// VKの初期化が終わったらシグナルを接続
		connect(m_vulkanWindow, &VulkanWindow::sigInitFinished, this, [this] { connectDockWidgetsSignals(); });
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

	void MainWindow::mousePressEvent(QMouseEvent* event)
	{
		m_vulkanWindow->mousePressEvent(event);
		__super::mousePressEvent(event);
	}

	void MainWindow::mouseReleaseEvent(QMouseEvent* event)
	{
		m_vulkanWindow->mouseReleaseEvent(event);
		__super::mouseReleaseEvent(event);
	}

	void MainWindow::connectDockWidgetsSignals()
	{
		// VKの初期化が終わったらシグナルを接続

		// エディタ閉じる
		connect(m_vulkanWindow, &VulkanWindow::sigCloseWindow, this, &MainWindow::closeWindow);

		// GameStageファイル開く
		connect(m_contentBrowser, &ContentBrowserDockWidget::sigOpenFile, this, &MainWindow::openFile);

		// MObject追加
		connect(m_contentBrowser, &ContentBrowserDockWidget::sigAddMObject, this, [this]
			{
				MObject_ptr newObject = m_vulkanWindow->GetEditorGameDirector()->AddNewMObject();	// GameDirectorでオブジェクト作成
				m_overview->AddTopLevelObject(newObject);
			});

		// Overviewの選択オブジェクト変更
		connect(m_overview, &OverviewDockWidget::sigChangeSelection, m_propertyViewer, &PropertyViewerDockWidget::setObject);

		// PropertyViewerで Component追加
		connect(m_propertyViewer, &PropertyViewerDockWidget::sigAddComponent, this, [this](const uuids::uuid& uuid, const ComponentType type)
			{
				m_vulkanWindow->GetEditorGameDirector()->RegisterComponent(uuid, type);
			});
	}

	void MainWindow::closeWindow()
	{
		close();
		ThreadPool::Instance().Shutdown();
	}

	void MainWindow::openFile(std::string filePath)
	{
		m_vulkanWindow->OpenFile(filePath);
	}

	void MainWindow::resetLayout()
	{
		if (m_contentBrowser->isHidden()) m_contentBrowser->show();
		if (m_logger->isHidden()) m_logger->show();
		if (m_overview->isHidden()) m_overview->show();
		if (m_propertyViewer->isHidden()) m_propertyViewer->show();
		m_contentBrowser->setFloating(false);
		m_logger->setFloating(false);
		m_overview->setFloating(false);
		m_propertyViewer->setFloating(false);
		resizeDocks({ m_contentBrowser, m_logger }, { 300, 150 }, Qt::Orientation::Horizontal);
		resizeDocks({ m_contentBrowser, m_logger }, { 250, 250 }, Qt::Orientation::Vertical);
		resizeDocks({ m_overview, m_propertyViewer }, { 250, 250 }, Qt::Orientation::Vertical);
		resizeDocks({ m_overview, m_propertyViewer }, { 300, 300 }, Qt::Orientation::Horizontal);
	}
}