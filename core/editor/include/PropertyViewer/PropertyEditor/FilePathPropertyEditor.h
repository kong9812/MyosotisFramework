// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"

/**
 * @brief ファイルパス用プロパティエディタ
 * データ本体(FilePath.path)は常に「絶対パス」で保持しますが、
 * UI(QLineEdit)上では baseDir を起点とした「相対パス」で表示・編集を行います。
 */
class FilePathPropertyEditor : public PropertyEditorBase
{
public:
	FilePathPropertyEditor(void* obj,
		const MyosotisFW::PropertyDesc& desc,
		QWidget* parent,
		const QString& baseDir = QString(),
		const QString& filter = QObject::tr("All Files (*.*)"),
		const QString& defaultDir = QString()) :
		PropertyEditorBase(obj, desc, parent),
		m_lineEdit(new QLineEdit(this)),
		m_browseButton(new QPushButton("...", this)),
		m_baseDir(baseDir),
		m_filter(filter),
		m_defaultDir(defaultDir)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(2);

		m_lineEdit->setPlaceholderText("No file selected...");
		m_browseButton->setFixedWidth(30);

		// ブラウズボタンクリック
		connect(m_browseButton, &QPushButton::clicked, [this]()
			{
				const QString start = getDialogStartDir();
				QString fileName = QFileDialog::getOpenFileName(this,
					tr("Select File"),
					start,
					m_filter);

				if (!fileName.isEmpty())
				{
					// 常に絶対パスに変換して適用
					applyAbsolutePath(fileName);
					refreshUI();
				}
			});

		// 手入力でEnterを押した時の処理
		connect(m_lineEdit, &QLineEdit::editingFinished, [this]()
			{
				if (m_lineEdit->isModified())
				{
					// UI上で相対パスが入力されても絶対パスに直して適用
					applyAbsolutePath(m_lineEdit->text());
					refreshUI();
					m_lineEdit->setModified(false);
				}
			});

		layout->addWidget(m_lineEdit);
		layout->addWidget(m_browseButton);
		refreshUI();
	}

	void refreshUI() override
	{
		// PropertyDesc から値(絶対パス)を取得
		MyosotisFW::PropertyDesc::PropertyValue valVariant = m_desc.get(m_object);
		if (std::holds_alternative<MyosotisFW::FilePath>(valVariant))
		{
			MyosotisFW::FilePath fp = std::get<MyosotisFW::FilePath>(valVariant);

			// 絶対パスを表示用の相対パスに変換
			QString displayStr = toDisplayPath(QString::fromStdString(fp.path));

			m_lineEdit->blockSignals(true);
			m_lineEdit->setText(displayStr);
			m_lineEdit->setModified(false);
			m_lineEdit->blockSignals(false);
		}
	}

private:
	void applyAbsolutePath(const QString& inputPath)
	{
		QFileInfo fi = fileInfoResolved(inputPath);

		MyosotisFW::FilePath fp{};
		// 内部データは常に絶対パス（OS標準の区切り文字）
		fp.path = QDir::toNativeSeparators(fi.absoluteFilePath()).toStdString();

		m_desc.apply(m_object, fp, MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
	}

	QString toDisplayPath(const QString& fullPath) const
	{
		if (fullPath.isEmpty() || m_baseDir.isEmpty()) return fullPath;

		QDir base(m_baseDir);
		QFileInfo fi(fullPath);

		if (fi.isAbsolute())
		{
			// baseDir配下なら相対パスを取得
			QString rel = base.relativeFilePath(fullPath);
			// フォルダ外(../../)でなければ相対パスを優先
			if (!rel.startsWith(".."))
			{
				return QDir::toNativeSeparators(rel);
			}
		}
		return QDir::toNativeSeparators(fullPath);
	}

	QString getDialogStartDir() const
	{
		QString p = m_lineEdit->text().trimmed();
		if (!p.isEmpty())
		{
			QFileInfo fi = fileInfoResolved(p);
			if (fi.exists()) {
				return fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
			}
		}

		if (!m_defaultDir.isEmpty() && QDir(m_defaultDir).exists()) return m_defaultDir;
		if (!m_baseDir.isEmpty() && QDir(m_baseDir).exists()) return m_baseDir;

		return QDir::currentPath();
	}

	QFileInfo fileInfoResolved(const QString& path) const
	{
		QFileInfo fi(path);
		// 相対パスで、かつ baseDir が設定されていれば結合
		if (fi.isRelative() && !m_baseDir.isEmpty())
		{
			QDir base(m_baseDir);
			return QFileInfo(base.absoluteFilePath(path));
		}
		return fi;
	}

private:
	QLineEdit* m_lineEdit;
	QPushButton* m_browseButton;

	QString m_baseDir;
	QString m_filter;
	QString m_defaultDir;
};