// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

class ContentBrowserFileSystemModel : public QFileSystemModel
{
public:
	ContentBrowserFileSystemModel(std::string path, QObject* parent = nullptr) : QFileSystemModel(parent)
	{
		QFileInfo rootPath(path.c_str());

		// TreeにRootPathを表示するため、RootPathの親をRootとしてセットする
		QDir parentPath = rootPath.absolutePath();
		parentPath.cdUp();
		setRootPath(parentPath.path());
	}
};