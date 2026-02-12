// Copyright (c) 2025 kong9812
#include "OverviewDockWidget.h"
#include "AppInfo.h"
#include "Logger.h"

using namespace MyosotisFW;

OverviewDockWidget::OverviewDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_treeView(new OverviewTreeView(m_mainWidget)),
	m_MObjectModel(new OverviewMObjectModel(m_treeView)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget))
{
	std::vector<MObject_ptr> empty{};
	m_MObjectModel->setTopLevelObjects(empty);

	m_treeView->setModel(m_MObjectModel);
	m_treeView->setHeaderHidden(true);
	m_treeView->expandAll();
	connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		this, [this](const QModelIndex& current, const QModelIndex& previous)
		{
			if (!current.isValid())
			{
				emit sigChangeSelection(nullptr);
				return;
			}

			MObject* obj = static_cast<MObject*>(current.internalPointer());
			emit sigChangeSelection(obj);
		});
	connect(m_treeView, &QTreeView::doubleClicked,
		this, [this](const QModelIndex& index)
		{
			if (!index.isValid()) return;

			MObject* obj = static_cast<MObject*>(index.internalPointer());
			emit sigFocusObject(obj);
		});

	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->addWidget(m_treeView);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Content Browser");
	setWidget(m_mainWidget);
}

void OverviewDockWidget::Clear()
{
	m_MObjectModel->ClearAllData();
}

void OverviewDockWidget::Reload()
{
	m_MObjectModel->layoutChanged();
	m_treeView->viewport()->update();
}

void OverviewDockWidget::AddTopLevelObject(MObject_ptr topObject)
{
	QModelIndex newIndex = m_MObjectModel->addObject(QModelIndex(), topObject);
	if (newIndex.isValid())
	{
		m_treeView->setCurrentIndex(newIndex);
	}
}

void OverviewDockWidget::SetTopLevelObjects(std::vector<MObject_ptr> topObjects)
{
	m_MObjectModel->setTopLevelObjects(topObjects);
}

void OverviewDockWidget::SetSelection(MObject* object)
{
	if (!object)
	{
		m_treeView->clearSelection();
		m_treeView->setCurrentIndex(QModelIndex());
		return;
	}
	QModelIndex targetIndex = m_MObjectModel->indexFromObject(object);
	if (targetIndex.isValid())
	{
		m_treeView->scrollTo(targetIndex);
		m_treeView->setCurrentIndex(targetIndex);
	}
}