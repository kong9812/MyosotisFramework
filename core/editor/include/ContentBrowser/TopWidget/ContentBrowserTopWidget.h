// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ContentBrowserTopWidgetAddButton.h"
#include "ContentBrowserTopWidgetBackButton.h"
#include "ContentBrowserTopWidgetForwardButton.h"
#include "ContentBrowserTopWidgetImportButton.h"
#include "ContentBrowserTopWidgetSaveAllButton.h"
#include "ContentBrowserTopWidgetSettingButton.h"

class ContentBrowserTopWidget : public QWidget
{
	Q_OBJECT
public:
	ContentBrowserTopWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private:
	QHBoxLayout* m_hLayout;

	ContentBrowserTopWidgetAddButton* m_addButton;
	ContentBrowserTopWidgetBackButton* m_backButton;
	ContentBrowserTopWidgetForwardButton* m_forwardButton;
	ContentBrowserTopWidgetImportButton* m_importButton;
	ContentBrowserTopWidgetSaveAllButton* m_saveAllButton;
	ContentBrowserTopWidgetSettingButton* m_settingButton;

Q_SIGNALS:
	void addMObject();

};