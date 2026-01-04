// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class ContentBrowserTopWidgetAddButton : public QPushButton
{
	Q_OBJECT
public:
	ContentBrowserTopWidgetAddButton(QWidget* parent = nullptr);

private:
	QMenu* m_menu;

Q_SIGNALS:
	void addMObject();

};