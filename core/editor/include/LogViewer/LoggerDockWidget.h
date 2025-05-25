// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "LoggerPreviewModel.h"
#include "LoggerPreviewItemDelegate.h"
#include "LoggerPreviewFilterProxyModel.h"

class LoggerDockWidget : public QDockWidget
{
public:
	LoggerDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

private:
	QWidget* m_mainWidget;
	QVBoxLayout* m_vBoxLayout;

	QFileSystemWatcher* m_fileSystemWatcher;

	QLineEdit* m_lineEdit;

	QListView* m_listView;
	LoggerPreviewModel* m_loggerPreviewModel;
	LoggerPreviewItemDelegate* m_loggerPreviewItemDelegate;
	LoggerPreviewFilterProxyModel* m_loggerPreviewFilterProxyModel;
private:
	qint64 m_lastFileSize;

	void readFile();
	void lineEditTextChanged(const QString& newText);
};
