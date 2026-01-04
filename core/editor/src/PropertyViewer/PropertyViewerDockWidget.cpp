// Copyright (c) 2025 kong9812
#include "PropertyViewerDockWidget.h"
#include "AppInfo.h"
#include "Logger.h"
#include "MObject.h"

using namespace MyosotisFW;

PropertyViewerDockWidget::PropertyViewerDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget))
{

	m_name = new QLabel("", m_mainWidget);

	m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->addWidget(m_name);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Property Viewer");
	setWidget(m_mainWidget);
}

void PropertyViewerDockWidget::setObject(MObject* object)
{
	m_name->setText(object->GetName().c_str());
}