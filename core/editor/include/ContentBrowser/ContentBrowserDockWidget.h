// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

#include "ContentBrowserTopWidget.h"

#include "ContentBrowserSortFilterProxyModel.h"
#include "ContentBrowserFileSystemModel.h"

class ContentBrowserDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	ContentBrowserDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

private:
	QWidget* m_mainWidget;

	ContentBrowserFileSystemModel* m_fileSystemModel;
	ContentBrowserSortFilterProxyModel* m_sortFilterProxyModel;

	ContentBrowserTopWidget* m_topWidget;
	QTreeView* m_treeView;
	QListView* m_listView;

	QSplitter* m_splitter;
	QVBoxLayout* m_vBoxLayout;
	QHBoxLayout* m_hLayout;

Q_SIGNALS:
	void addMObject();
	void openFile(std::string filePath);
};