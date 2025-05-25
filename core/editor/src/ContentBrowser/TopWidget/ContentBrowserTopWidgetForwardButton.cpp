// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetForwardButton.h"

ContentBrowserTopWidgetForwardButton::ContentBrowserTopWidgetForwardButton(QWidget* parent) :
	QPushButton(parent)
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_ArrowForward));
}