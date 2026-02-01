// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include "iqt.h"
#include "SkyboxSettingWidget.h"
#include "FilePath.h"

class WorldSettingDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	WorldSettingDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

private:
	QWidget* m_mainWidget;

	QVBoxLayout* m_vBoxLayout;
	SkyboxSettingWidget* m_skyboxSettingWidget;

Q_SIGNALS:
	void sigSkyboxCubemapChanged(const std::array<MyosotisFW::FilePath, 6>& filePath);

};