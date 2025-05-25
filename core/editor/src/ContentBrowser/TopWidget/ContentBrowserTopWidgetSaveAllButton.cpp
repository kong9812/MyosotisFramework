// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetSaveAllButton.h"

ContentBrowserTopWidgetSaveAllButton::ContentBrowserTopWidgetSaveAllButton(QWidget* parent) :
	QPushButton(parent)
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_DialogSaveAllButton));
	setText("Save All");
}