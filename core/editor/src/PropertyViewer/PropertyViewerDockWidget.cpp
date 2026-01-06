// Copyright (c) 2025 kong9812
#include "PropertyViewerDockWidget.h"
#include "AppInfo.h"
#include "Logger.h"
#include "MObject.h"
#include "ComponentType.h"
#include "ComponentBase.h"

using namespace MyosotisFW;

PropertyViewerDockWidget::PropertyViewerDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget)),
	m_name(new QLabel("", m_mainWidget)),
	m_uuid(new QLabel("", m_mainWidget)),
	m_dummy(new QLabel("", m_mainWidget)),
	m_addComponentButton(new QPushButton(m_mainWidget)),
	m_addComponentMenu(new QMenu(m_mainWidget)),
	m_currentObject(nullptr)
{
	for (uint32_t i = static_cast<uint32_t>(ComponentType::Begin); i < static_cast<uint32_t>(ComponentType::Max); i++)
	{
		ComponentType componentType = static_cast<ComponentType>(i);
		m_addComponentMenu->addAction(std::string(ComponentTypeName[i]).c_str(), [this, componentType]() { addComponent(componentType); });
	}
	m_addComponentButton->setText(" + ");
	m_addComponentButton->setMenu(m_addComponentMenu);
	m_addComponentButton->setDisabled(true);

	m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->addWidget(m_name);
	m_vBoxLayout->addWidget(m_uuid);
	m_vBoxLayout->addWidget(m_dummy);
	m_vBoxLayout->addWidget(m_addComponentButton);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Property Viewer");
	setWidget(m_mainWidget);
}

void PropertyViewerDockWidget::UpdateView()
{
	setObject(m_currentObject);
}

void PropertyViewerDockWidget::addComponent(const MyosotisFW::ComponentType type)
{
	emit sigAddComponent(m_currentObject->GetUUID(), type);
	UpdateView();
}

void PropertyViewerDockWidget::setObject(MObject* object)
{
	m_currentObject = object;
	m_addComponentButton->setDisabled(m_currentObject == nullptr);

	if (m_currentObject)
	{
		m_name->setText(object->GetName().c_str());
		m_uuid->setText(uuids::to_string(object->GetUUID()).c_str());

		std::string components = "Components:\n";
		for (ComponentBase_ptr component : object->GetAllComponents())
		{
			components += ComponentTypeName[static_cast<uint32_t>(component->GetType())] + std::string("\n");
		}
		m_dummy->setText(components.c_str());
	}
}