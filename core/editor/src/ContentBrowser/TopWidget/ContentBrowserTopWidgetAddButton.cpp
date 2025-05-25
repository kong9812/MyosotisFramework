// Copyright (c) 2025 kong9812
#include "ContentBrowserTopWidgetAddButton.h"
#include "Logger.h"

ContentBrowserTopWidgetAddButton::ContentBrowserTopWidgetAddButton(QWidget* parent) :
	QPushButton(parent),
	m_menu(new QMenu(this))
{
	setIcon(style()->standardIcon(QStyle::StandardPixmap::SP_FileIcon));
	setText("Add");

	m_menu->addAction("FPSCamera", []() { Logger::Debug("Add FPSCamera!"); });
	m_menu->addAction("PrimitiveGeometryMesh", []() { Logger::Debug("Add PrimitiveGeometryMesh!"); });
	m_menu->addAction("CustomMesh", []() { Logger::Debug("Add CustomMesh!"); });
	m_menu->addAction("Skybox", []() { Logger::Debug("Add Skybox!"); });
	m_menu->addAction("InteriorObjectMesh", []() { Logger::Debug("Add InteriorObjectMesh!"); });

	setMenu(m_menu);
}