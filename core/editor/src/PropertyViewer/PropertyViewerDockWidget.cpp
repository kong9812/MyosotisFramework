// Copyright (c) 2025 kong9812
#include "PropertyViewerDockWidget.h"
#include "AppInfo.h"
#include "Logger.h"
#include "MObject.h"
#include "ComponentType.h"
#include "ComponentBase.h"

#include "PropertyViewerComponentWidget.h"

using namespace MyosotisFW;

PropertyViewerDockWidget::PropertyViewerDockWidget(QWidget* parent, Qt::WindowFlags flags) :
	QDockWidget(parent, flags),
	m_mainWidget(new QWidget(this)),
	m_scrollArea(new QScrollArea(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget)),
	m_name(new QLabel("", m_mainWidget)),
	m_uuid(new QLabel("", m_mainWidget)),
	m_dummy(new QLabel("", m_mainWidget)),
	m_addComponentButton(new QPushButton(m_mainWidget)),
	m_addComponentMenu(new QMenu(m_mainWidget)),
	m_currentObject(nullptr),
	m_componentWidgets()
{
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);

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

	m_scrollArea->setWidget(m_mainWidget);

	setContentsMargins(0, 0, 0, 0);
	setWindowTitle("Property Viewer");
	setWidget(m_scrollArea);
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

void PropertyViewerDockWidget::ClearFormLayout()
{
	for (QWidget* widget : m_componentWidgets)
	{
		widget->deleteLater();
	}
	m_componentWidgets.clear();
}

void PropertyViewerDockWidget::setObject(MObject* object)
{
	ClearFormLayout();

	m_currentObject = object;
	m_addComponentButton->setDisabled(m_currentObject == nullptr);

	if (m_currentObject)
	{
		m_name->setText(object->GetName().c_str());
		m_uuid->setText(uuids::to_string(object->GetUUID()).c_str());

		std::string components = "Components:\n";


		for (ComponentBase_ptr component : *object->GetAllComponents())
		{
			PropertyViewerComponentWidget* componentWidget = new PropertyViewerComponentWidget(ComponentTypeName[static_cast<uint32_t>(component->GetType())]);
			componentWidget->SetComponent(component);
			m_componentWidgets.push_back(componentWidget);
			m_vBoxLayout->addWidget(componentWidget);
		}
		m_dummy->setText(components.c_str());
	}
}

// todo. 1つのQWidget 1つのComponent
// todo. std::vector<QWidget> m_componentWidgets;
// todo. 解放する時はQWidgetのdeleteLaterで全開放