// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class ContentBrowserSortFilterProxyModel : public QSortFilterProxyModel
{
public:
	ContentBrowserSortFilterProxyModel(const std::string& dir, QAbstractItemModel* sourceModel, QObject* parent = nullptr) : QSortFilterProxyModel(parent), m_rootDir(dir)
	{
		setSourceModel(sourceModel);
	}

protected:
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override
	{
		// ディレクトリのみを許可
		QFileSystemModel* fsModel = qobject_cast<QFileSystemModel*>(sourceModel());

		// 元のモデルのインデックスを取得
		QModelIndex index = mapToSource(parent);

		std::string str = fsModel->filePath(index).toUtf8().constData();
		if (fsModel->isDir(index))
		{
			QDir dir(str.c_str());
			QFileInfoList fileInfoList = dir.entryInfoList();

			foreach(const QFileInfo & fileInfo, fileInfoList)
			{
				if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

				if (fileInfo.isDir()) return true;
			}
		}
		return false;
	}

	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
	{
		// 元のモデルのインデックスを取得
		QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

		// ディレクトリのみを許可
		QFileSystemModel* fsModel = qobject_cast<QFileSystemModel*>(sourceModel());

		// モデルがない
		if (!fsModel) return false;

		// ディレクトリじゃない
		if (!fsModel->isDir(index)) return false;

		// ルート以下のディレクトじゃない
		QDir rootParentDir = QDir(m_rootDir.c_str());
		rootParentDir.cdUp();	// ルートを表示するため、ルートの親もOKにする必要がある
		std::string currentDirStr = fsModel->filePath(index).toUtf8().constData();
		std::string rootParentDirStr = rootParentDir.absolutePath().toUtf8().constData();
		bool result = fsModel->filePath(index).startsWith(QDir(m_rootDir.c_str()).path(), Qt::CaseSensitivity::CaseInsensitive);
		if ((!fsModel->filePath(index).startsWith(QDir(m_rootDir.c_str()).path(), Qt::CaseSensitivity::CaseInsensitive)) &&
			(currentDirStr != rootParentDirStr))
		{
			return false;
		}
		return true;
	}

private:
	std::string m_rootDir;
};