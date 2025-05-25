// Copyright (c) 2025 kong9812
#include "LoggerDockWidget.h"
#include "Logger.h"

LoggerDockWidget::LoggerDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget)),
	m_fileSystemWatcher(new QFileSystemWatcher(m_mainWidget)),
	m_lineEdit(new QLineEdit(m_mainWidget)),
	m_listView(new QListView(m_mainWidget)),
	m_loggerPreviewModel(new LoggerPreviewModel(m_mainWidget)),
	m_loggerPreviewItemDelegate(new LoggerPreviewItemDelegate(m_mainWidget)),
	m_loggerPreviewFilterProxyModel(new LoggerPreviewFilterProxyModel(m_mainWidget)),
	m_lastFileSize(0)
{
	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Logger");
	setWidget(m_mainWidget);

	m_fileSystemWatcher->addPath(Logger::GetLogFileName().c_str());
	connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LoggerDockWidget::readFile);

	readFile();

	connect(m_lineEdit, &QLineEdit::textChanged, this, &LoggerDockWidget::lineEditTextChanged);

	m_loggerPreviewFilterProxyModel->setSourceModel(m_loggerPreviewModel);
	m_listView->setModel(m_loggerPreviewFilterProxyModel);
	m_listView->setItemDelegate(m_loggerPreviewItemDelegate);
	m_vBoxLayout->addWidget(m_lineEdit);
	m_vBoxLayout->addWidget(m_listView);
	m_vBoxLayout->addLayout(m_vBoxLayout);
}

void LoggerDockWidget::readFile()
{
	QFile file(Logger::GetLogFileName().c_str());

	if (!file.exists())
	{
		ASSERT(file.open(QIODevice::WriteOnly | QIODevice::Text), "Failed to create new log file.");
		file.close();
	}
	ASSERT(file.open(QIODevice::ReadOnly | QIODevice::Text), "Failed to open log file.");

	auto size = file.size();

	if (file.size() > m_lastFileSize)
	{
		file.seek(m_lastFileSize);
		QTextStream textStream(&file);
		while (!textStream.atEnd())
		{
			m_loggerPreviewModel->AddLog(textStream.readLine());
		}
		m_lastFileSize = file.size();
	}
	file.close();
}

void LoggerDockWidget::lineEditTextChanged(const QString& newText)
{
	m_loggerPreviewFilterProxyModel->setFilterRegularExpression(QRegularExpression(newText, QRegularExpression::PatternOption::CaseInsensitiveOption));
}

