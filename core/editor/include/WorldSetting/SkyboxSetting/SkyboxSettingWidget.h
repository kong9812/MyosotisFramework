// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include "iqt.h"
#include "FilePath.h"

class SkyboxSettingWidget : public QWidget
{
	Q_OBJECT
public:
	SkyboxSettingWidget(QWidget* parent = nullptr);

private:
	QVBoxLayout* m_vBoxLayout;
	MyosotisFW::FilePath m_filePath;

Q_SIGNALS:
	void sigSkyboxCubemapChanged(const std::array<MyosotisFW::FilePath, 6>& filePath);

};