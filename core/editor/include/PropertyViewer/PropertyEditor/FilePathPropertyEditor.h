// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"

// wip. 引数でデフォルトフォルダ指定できるようにする
// FilePathに親フォルダ指定できるようにする？？

class FilePathPropertyEditor : public PropertyEditorBase
{
public:
	FilePathPropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent, const std::string& baseDir = "") :
		PropertyEditorBase(obj, desc, parent),
		m_lineEdit(new QLineEdit(this)),
		m_browseButton(new QPushButton("...", this)),
		m_baseDir(baseDir)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(2);

		m_lineEdit->setPlaceholderText("No file selected...");
		m_browseButton->setFixedWidth(30);

		connect(m_browseButton, &QPushButton::clicked, [this]()
			{
				QString currentPath = m_lineEdit->text();
				QString fileName = QFileDialog::getOpenFileName(this,
					tr("Select File"),
					currentPath,
					tr("All Files (*.*)")); // todo. リソースのフォルダに指定 (引数で？)

				if (!fileName.isEmpty())
				{
					MyosotisFW::FilePath fp{};
					fp.path = QDir::toNativeSeparators(fileName).toStdString();

					m_desc.apply(m_object, fp, MyosotisFW::ChangeReason::UI_Preview);
					refreshUI(); // 自身の表示を更新
				}
			});

		// Enterを押した時の処理
		connect(m_lineEdit, &QLineEdit::editingFinished, [this]()
			{
				if (m_lineEdit->isModified())
				{
					MyosotisFW::FilePath fp;
					fp.path = m_lineEdit->text().toStdString();
					m_desc.apply(m_object, fp, MyosotisFW::ChangeReason::UI_Preview);
					m_lineEdit->setModified(false);
				}
			});

		layout->addWidget(m_lineEdit);
		layout->addWidget(m_browseButton);
		refreshUI();
	}

	void refreshUI() override
	{
		// PropertyDesc から値を取得
		MyosotisFW::PropertyValue valVariant = m_desc.get(m_object);
		if (std::holds_alternative<MyosotisFW::FilePath>(valVariant))
		{
			MyosotisFW::FilePath fp = std::get<MyosotisFW::FilePath>(valVariant);

			m_lineEdit->blockSignals(true);
			m_lineEdit->setText(QString::fromStdString(fp.path));
			m_lineEdit->setModified(false);
			m_lineEdit->blockSignals(false);
		}
	}

private:
	QLineEdit* m_lineEdit;
	QPushButton* m_browseButton;
	std::string m_baseDir;
};