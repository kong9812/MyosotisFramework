// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class ContentBrowserTopWidgetAddButton : public QPushButton
{
public:
	ContentBrowserTopWidgetAddButton(QWidget* parent = nullptr);

private:
	QMenu* m_menu;
};