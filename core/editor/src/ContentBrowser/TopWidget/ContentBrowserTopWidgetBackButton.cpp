// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetBackButton.h"

ContentBrowserTopWidgetBackButton::ContentBrowserTopWidgetBackButton(QWidget* parent) :
	QPushButton(parent)
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_ArrowBack));
}