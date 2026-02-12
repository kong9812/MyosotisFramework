// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class OverviewTreeView : public QTreeView
{
public:
	OverviewTreeView(QWidget* parent = nullptr) : QTreeView(parent) {}

protected:
	void mousePressEvent(QMouseEvent* e) override;

};