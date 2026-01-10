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
	m_scrollArea(new QScrollArea(this)),
	m_vBoxLayout(new QVBoxLayout(m_mainWidget)),
	m_name(new QLabel("", m_mainWidget)),
	m_uuid(new QLabel("", m_mainWidget)),
	m_dummy(new QLabel("", m_mainWidget)),
	m_formLayout(new QFormLayout(this)),
	m_groupBox(new QGroupBox(this)),
	m_addComponentButton(new QPushButton(m_mainWidget)),
	m_addComponentMenu(new QMenu(m_mainWidget)),
	m_currentObject(nullptr),
	m_propertyEditorFactory(std::make_unique<PropertyEditorFactory>())
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

	m_formLayout->setLabelAlignment(Qt::AlignmentFlag::AlignLeft);
	m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	m_formLayout->setSpacing(0);
	m_formLayout->setContentsMargins(0, 0, 0, 0);

	m_groupBox->setWindowIconText("TEST");

	m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
	m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_vBoxLayout->addWidget(m_name);
	m_vBoxLayout->addWidget(m_uuid);
	m_vBoxLayout->addWidget(m_dummy);
	m_vBoxLayout->addLayout(m_formLayout);
	m_vBoxLayout->addWidget(m_addComponentButton);
	m_vBoxLayout->addWidget(m_groupBox);

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
	if (!m_formLayout) return;
	while (m_formLayout->count() > 0)
	{
		QLayoutItem* item = m_formLayout->takeAt(0);
		if (QWidget* widget = item->widget())
		{
			widget->deleteLater();
		}
		delete item;
	}
}

void PropertyViewerDockWidget::setObject(MObject* object)
{
	m_activeEditors.clear();
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
			components += ComponentTypeName[static_cast<uint32_t>(component->GetType())] + std::string("\n");

			// property 表示
			PropertyTable propertyTable = component->GetPropertyTable();
			propertyTable.ForEach([&](const PropertyDesc& desc)
				{
					//components += std::string("ID: ") + uuids::to_string(desc.id) + "\n";
					//components += std::string("type: ") + desc.type.name + "(" + uuids::to_string(desc.type.id) + ")" + "\n";
					//components += std::string("category: ") + desc.category + "\n";
					//components += std::string("name: ") + desc.name + "\n";
					//components += "\n";
					std::unique_ptr<PropertyEditorBase> editorWidget = m_propertyEditorFactory->CreateEditor(component.get(), desc, m_mainWidget);
					if (editorWidget)
					{
						PropertyEditorBase* ptr = editorWidget.get();
						m_activeEditors.emplace_back(std::move(editorWidget));
						m_formLayout->addRow(desc.name, ptr);
					}
				});

		}
		m_dummy->setText(components.c_str());
	}
}

// todo. 1つのQWidget 1つのComponent
// todo. std::vector<QWidget> m_componentWidgets;
// todo. 解放する時はQWidgetのdeleteLaterで全開放