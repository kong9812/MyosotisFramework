// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetSettingButton.h"

ContentBrowserTopWidgetSettingButton::ContentBrowserTopWidgetSettingButton(QWidget* parent) :
	QPushButton(parent)
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_FileDialogDetailedView));
	setText("Setting");
}