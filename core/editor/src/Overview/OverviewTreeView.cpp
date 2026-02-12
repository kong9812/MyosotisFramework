// Copyright (c) 2025 kong9812
#include "OverviewTreeView.h"

void OverviewTreeView::mousePressEvent(QMouseEvent* e)
{
	if (!indexAt(e->pos()).isValid())
	{
		clearSelection();
		setCurrentIndex(QModelIndex());
	}
	__super::mousePressEvent(e);
}