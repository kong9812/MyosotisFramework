// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "OverviewMObjectModel.h"

class OverviewDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	OverviewDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	void AddTopLevelObject(MObject_ptr topObject);
	void SetTopLevelObjects(std::vector<MObject_ptr> topObjects);

private:
	QWidget* m_mainWidget;

	QTreeView* m_treeView;
	OverviewMObjectModel* m_MObjectModel;

	QVBoxLayout* m_vBoxLayout;

Q_SIGNALS:
	void changeSelection(MObject* object);

};