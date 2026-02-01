// Copyright (c) 2025 kong9812
#include "WorldSettingDockWidget.h"

WorldSettingDockWidget::WorldSettingDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget)),
	m_skyboxSettingWidget(new SkyboxSettingWidget(m_mainWidget))
{
	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
	m_vBoxLayout->addWidget(m_skyboxSettingWidget);

	connect(m_skyboxSettingWidget, &SkyboxSettingWidget::sigSkyboxCubemapChanged, this, &WorldSettingDockWidget::sigSkyboxCubemapChanged);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("World Setting");
	setWidget(m_mainWidget);
}