// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class LoggerPreviewModel : public QAbstractListModel
{
public:
	LoggerPreviewModel(QObject* parent = nullptr) : QAbstractListModel(parent), m_logList() {}
	int rowCount(const QModelIndex& parent = QModelIndex()) const override {
		Q_UNUSED(parent);
		return m_logList.size();
	}

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
		if (!index.isValid() || index.row() >= m_logList.size()) return QVariant();
		if (role == Qt::DisplayRole) return m_logList[index.row()];
		return QVariant();
	}

	void AddLog(QString log)
	{
		beginInsertRows(QModelIndex(), 0, 0);
		m_logList.push_front(log);
		endInsertRows();
	}

private:
	QStringList m_logList;
};