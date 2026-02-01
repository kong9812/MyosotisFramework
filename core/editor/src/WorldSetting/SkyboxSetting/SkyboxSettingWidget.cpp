// Copyright (c) 2025 kong9812
#include "SkyboxSettingWidget.h"
#include "DirectoryPathEditor.h"
#include "AppInfo.h"
#include "Logger.h"

SkyboxSettingWidget::SkyboxSettingWidget(QWidget* parent) :
	QWidget(parent),
	m_vBoxLayout(new QVBoxLayout(this)),
	m_filePath()
{
	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);

	QLabel* cubemapDirectoryPathLabel = new QLabel(this);
	cubemapDirectoryPathLabel->setText("Cube map Directory:");
	m_vBoxLayout->addWidget(cubemapDirectoryPathLabel);

	DirectoryPathEditor* filePathEditor = new DirectoryPathEditor(this, &m_filePath, MyosotisFW::AppInfo::g_assetRootFolder, MyosotisFW::AppInfo::g_textureFolder);
	m_vBoxLayout->addWidget(filePathEditor);

	QListWidget* imageList = new QListWidget(this);
	imageList->setViewMode(QListWidget::IconMode);
	imageList->setIconSize(QSize(64, 64));
	imageList->setDragDropMode(QAbstractItemView::NoDragDrop);
	imageList->setFixedHeight(120);
	m_vBoxLayout->addWidget(imageList);

	auto emitCurrentOrder = [this, imageList]()
		{
			if (imageList->count() != 6) return;

			std::array<MyosotisFW::FilePath, 6> paths;
			for (uint32_t i = 0; i < 6; i++)
			{
				paths[i].path = imageList->item(i)->data(Qt::UserRole).toString().toStdString();
			}
			emit sigSkyboxCubemapChanged(paths);
			Logger::Debug("Skybox order updated and signaled.");
		};

	connect(filePathEditor, &DirectoryPathEditor::valueChanged, [this, imageList, emitCurrentOrder]()
		{
			imageList->clear();
			QDir dir(QString::fromStdString(m_filePath.path));
			QStringList filters = { "*.png" };
			QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files, QDir::Name);

			QStringList keywords = { "right", "left", "top", "bottom", "front", "back" };
			QFileInfoList sortedList;
			QFileInfoList remainingFiles = fileList;

			for (const QString& key : keywords)
			{
				for (uint32_t i = 0; i < remainingFiles.size(); i++)
				{
					if (remainingFiles[i].fileName().contains(key, Qt::CaseInsensitive))
					{
						sortedList.append(remainingFiles.takeAt(i));
						break;
					}
				}
			}
			sortedList.append(remainingFiles);

			for (const QFileInfo& fi : sortedList)
			{
				QListWidgetItem* item = new QListWidgetItem(QIcon(fi.absoluteFilePath()), fi.fileName());
				item->setData(Qt::UserRole, QDir::toNativeSeparators(fi.absoluteFilePath()));
				imageList->addItem(item);
			}

			if (imageList->count() == 6) emitCurrentOrder();
		});
	connect(imageList->model(), &QAbstractItemModel::rowsMoved, this, emitCurrentOrder);
}