// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class LoggerPreviewFilterProxyModel : public QSortFilterProxyModel
{
public:
	LoggerPreviewFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
	{
		QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
		QString data = index.data(Qt::DisplayRole).toString();
		return data.contains(filterRegularExpression());
	}
};