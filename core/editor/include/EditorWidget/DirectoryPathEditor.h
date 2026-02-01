// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "FilePath.h"

class DirectoryPathEditor : public QWidget
{
	Q_OBJECT
public:
	DirectoryPathEditor(
		QWidget* parent,
		MyosotisFW::FilePath* filePath,
		const QString& baseDir = QString(),
		const QString& defaultDir = QString(),
		const bool readOnly = false) :
		QWidget(parent),
		m_filePath(filePath),
		m_lineEdit(new QLineEdit(this)),
		m_browseButton(new QPushButton("...", this)),
		m_baseDir(baseDir),
		m_defaultDir(defaultDir)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(2);

		if (readOnly)
		{
			m_lineEdit->setReadOnly(true);
			m_lineEdit->setStyleSheet("QLineEdit { background: transparent; border: none; padding: 2px; }");
			m_lineEdit->setDisabled(true);
		}

		m_lineEdit->setPlaceholderText("No directory selected...");
		m_browseButton->setFixedWidth(30);

		connect(m_browseButton, &QPushButton::clicked, [this]()
			{
				const QString start = getDialogStartDir();
				QString dirName = QFileDialog::getExistingDirectory(this,
					tr("Select Directory"),
					start,
					QFileDialog::ShowDirsOnly);

				if (!dirName.isEmpty())
				{
					applyAbsolutePath(dirName);
					refreshUI();
				}
			});

		connect(m_lineEdit, &QLineEdit::editingFinished, [this]()
			{
				if (m_lineEdit->isModified())
				{
					applyAbsolutePath(m_lineEdit->text());
					refreshUI();
					m_lineEdit->setModified(false);
				}
			});

		layout->addWidget(m_lineEdit);
		layout->addWidget(m_browseButton);
		refreshUI();
	}

	void refreshUI()
	{
		QString displayStr = toDisplayPath(QString::fromStdString(m_filePath->path));
		m_lineEdit->blockSignals(true);
		m_lineEdit->setText(displayStr);
		m_lineEdit->setModified(false);
		m_lineEdit->blockSignals(false);
	}

private:
	void applyAbsolutePath(const QString& inputPath)
	{
		QFileInfo fi = fileInfoResolved(inputPath);
		m_filePath->path = QDir::toNativeSeparators(fi.absoluteFilePath()).toStdString();
		emit valueChanged();
	}

	QString toDisplayPath(const QString& fullPath) const
	{
		if (fullPath.isEmpty() || m_baseDir.isEmpty()) return QDir::toNativeSeparators(fullPath);
		QDir base(m_baseDir);
		if (QFileInfo(fullPath).isAbsolute())
		{
			QString rel = base.relativeFilePath(fullPath);
			if (!rel.startsWith("..")) return QDir::toNativeSeparators(rel);
		}
		return QDir::toNativeSeparators(fullPath);
	}

	QString getDialogStartDir() const
	{
		QString p = m_lineEdit->text().trimmed();
		if (!p.isEmpty())
		{
			QFileInfo fi = fileInfoResolved(p);
			if (fi.exists()) return fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
		}
		if (!m_defaultDir.isEmpty() && QDir(m_defaultDir).exists()) return m_defaultDir;
		if (!m_baseDir.isEmpty() && QDir(m_baseDir).exists()) return m_baseDir;
		return QDir::currentPath();
	}

	QFileInfo fileInfoResolved(const QString& path) const
	{
		QFileInfo fi(path);
		if (fi.isRelative() && !m_baseDir.isEmpty()) return QFileInfo(QDir(m_baseDir).absoluteFilePath(path));
		return fi;
	}

private:
	MyosotisFW::FilePath* m_filePath;
	QLineEdit* m_lineEdit;
	QPushButton* m_browseButton;
	QString m_baseDir;
	QString m_defaultDir;

signals:
	void valueChanged();
};