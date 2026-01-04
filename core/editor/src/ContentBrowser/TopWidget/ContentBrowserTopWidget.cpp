// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidget.h"

ContentBrowserTopWidget::ContentBrowserTopWidget(QWidget* parent, Qt::WindowFlags f) :
	QWidget(parent, f),
	m_hLayout(new QHBoxLayout(this)),
	m_addButton(new ContentBrowserTopWidgetAddButton(this)),
	m_backButton(new ContentBrowserTopWidgetBackButton(this)),
	m_forwardButton(new ContentBrowserTopWidgetForwardButton(this)),
	m_importButton(new ContentBrowserTopWidgetImportButton(this)),
	m_saveAllButton(new ContentBrowserTopWidgetSaveAllButton(this)),
	m_settingButton(new ContentBrowserTopWidgetSettingButton(this))
{
	setFixedHeight(30);
	m_hLayout->setContentsMargins(0, 0, 0, 0);
	m_hLayout->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignHCenter);
	m_hLayout->addWidget(m_addButton);
	m_hLayout->addWidget(m_backButton);
	m_hLayout->addWidget(m_forwardButton);
	m_hLayout->addWidget(m_importButton);
	m_hLayout->addWidget(m_saveAllButton);
	m_hLayout->addStretch();
	m_hLayout->addWidget(m_settingButton);

	setLayout(m_hLayout);

	connect(m_addButton, &ContentBrowserTopWidgetAddButton::addMObject, this, &ContentBrowserTopWidget::addMObject);
}