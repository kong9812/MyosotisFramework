// Copyright (c) 2025 kong9812
#include "ContentBrowserDockWidget.h"
#include "AppInfo.h"
#include "Logger.h"

ContentBrowserDockWidget::ContentBrowserDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_fileSystemModel(Q_NULLPTR),
	m_sortFilterProxyModel(Q_NULLPTR),
	m_topWidget(new ContentBrowserTopWidget(m_mainWidget)),
	m_treeView(new QTreeView(m_mainWidget)),
	m_listView(new QListView(m_mainWidget)),
	m_splitter(new QSplitter(m_mainWidget)),
	m_vBoxLayout(new QVBoxLayout()),
	m_hLayout(new QHBoxLayout())
{
	std::filesystem::path resourcesFolderPath = std::filesystem::absolute(MyosotisFW::AppInfo::g_resourcesFolder);

	// Common Model
	m_fileSystemModel = new ContentBrowserFileSystemModel(resourcesFolderPath.string(), m_mainWidget);

	// Tree View
	m_sortFilterProxyModel = new ContentBrowserSortFilterProxyModel(resourcesFolderPath.string(), m_fileSystemModel, m_mainWidget);
	m_treeView->setModel(m_sortFilterProxyModel);
	m_treeView->setHeaderHidden(true);
	m_treeView->hideColumn(1);
	m_treeView->hideColumn(2);
	m_treeView->hideColumn(3);
	m_treeView->setRootIndex(m_sortFilterProxyModel->mapFromSource(m_fileSystemModel->index(m_fileSystemModel->rootPath())));

	// ListView
	m_listView->setModel(m_fileSystemModel);
	m_listView->setRootIndex(m_fileSystemModel->index(resourcesFolderPath.string().c_str()));

	// QTreeViewの選択が変更されたときにQListViewを更新
	connect(m_treeView, &QTreeView::clicked, [&](const QModelIndex& index) {
		QModelIndex sourceModelIndex = m_sortFilterProxyModel->mapToSource(index);
		m_listView->setRootIndex(sourceModelIndex);
		});
	connect(m_listView, &QListView::doubleClicked, [&](const QModelIndex& index) {
		if (m_fileSystemModel->fileInfo(index).isDir())
		{
			m_listView->setRootIndex(index);
		}
		else
		{
			emit openFile(m_fileSystemModel->fileInfo(index).absoluteFilePath().toStdString());
		}
		});
	connect(m_topWidget, &ContentBrowserTopWidget::addMObject, this, &ContentBrowserDockWidget::addMObject);

	m_splitter->setContentsMargins(0, 0, 0, 0);
	m_splitter->addWidget(m_treeView);
	m_splitter->addWidget(m_listView);
	m_splitter->setSizes({ 200, 500 });
	m_hLayout->addWidget(m_splitter);

	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->addWidget(m_topWidget);
	m_vBoxLayout->addLayout(m_hLayout);

	m_mainWidget->setContentsMargins(0, 0, 0, 0);
	m_mainWidget->setLayout(m_vBoxLayout);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Content Browser");
	setWidget(m_mainWidget);
}
