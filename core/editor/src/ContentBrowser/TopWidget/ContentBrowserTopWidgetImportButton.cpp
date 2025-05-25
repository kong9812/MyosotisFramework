// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetImportButton.h"

ContentBrowserTopWidgetImportButton::ContentBrowserTopWidgetImportButton(QWidget* parent) :
	QPushButton(parent)
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_FileIcon));
	setText("Import");
}