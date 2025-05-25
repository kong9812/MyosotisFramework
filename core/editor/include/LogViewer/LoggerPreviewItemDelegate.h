// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class LoggerPreviewItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	LoggerPreviewItemDelegate(QObject* parent = nullptr) :QStyledItemDelegate(parent) {}

	void paint(QPainter* painter,
		const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		// data
		QString text = index.data(Qt::ItemDataRole::DisplayRole).toString();

		QLabel label(text);
		painter->save();
		painter->translate(option.rect.topLeft());
		label.render(painter);
		painter->restore();
	}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		Q_UNUSED(option);
		Q_UNUSED(index);
		return QSize(200, 20);
	}
};