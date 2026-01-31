// Copyright (c) 2025 kong9812
#include "VulkanWindow.h"
#include "AppInfo.h"
#include "VK_Validation.h"
#include "KeyConverter.h"
#include "ThreadPool.h"
#include "iimgui.h"
#include "VK_CreateInfo.h"
#include "MObjectRegistry.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, ImGuiIO& io); // Doesn't use ImGui::GetCurrentContext()

namespace MyosotisFW::System::Editor
{
	VulkanWindow::VulkanWindow(QWindow* parent) :
		QWindow(parent),
		m_instance(VK_NULL_HANDLE),
		m_surface(VK_NULL_HANDLE),
		m_renderSubsystem(nullptr),
		m_gameDirector(nullptr),
		m_initialized(false),
		m_resizing(false),
		m_lastTime(0.0f),
		m_timer(),
		m_mouseDragging(false),
		m_selectedObject(nullptr),
		m_statusBar(nullptr),
		m_mousePositionMonitor(new QLabel(""))
	{
		setTitle("VulkanWindow");
		setSurfaceType(QWindow::VulkanSurface);
		QCoreApplication::instance()->installEventFilter(this);
	}

	VulkanWindow::~VulkanWindow()
	{
		m_gameDirector.reset();
		m_renderSubsystem.reset();

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanWindow::SetStatusBar(QStatusBar* statusBar)
	{
		m_statusBar = statusBar;
		m_statusBar->addWidget(m_mousePositionMonitor, 1);
	}

	void VulkanWindow::Initialize()
	{
		// create vulkan instance
		VkApplicationInfo applicationInfo = Utility::Vulkan::CreateInfo::applicationInfo(
			AppInfo::g_applicationName,
			AppInfo::g_engineName,
			AppInfo::g_apiVersion,
			AppInfo::g_engineVersion);
		VkInstanceCreateInfo instanceCreateInfo = Utility::Vulkan::CreateInfo::instanceCreateInfo(
			applicationInfo,
			AppInfo::g_vkInstanceExtensionProperties,
			AppInfo::g_layer);
		VK_VALIDATION(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

		// create surface
#ifdef Q_OS_WIN
		HWND hwnd = reinterpret_cast<HWND>(winId());
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hwnd = hwnd;
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		VK_VALIDATION(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif

		// create render subsystem
		m_renderSubsystem = Render::CreateEditorRenderSubsystemPointer();
		m_renderSubsystem->SetObjectMovedCallback([this]() {emit sigObjectMoved(); });
		m_renderSubsystem->SetObjectSelectedCallback([this](MObject_ptr obj) {emit sigObjectSelected(obj); });
		m_renderSubsystem->Initialize(m_instance, m_surface);


		// Imgui
#ifdef _WIN32
		/*ASSERT(ImGui_ImplWin32_Init(hwnd), "Failed to init imgui win32.");
		ImGui::GetIO().WantCaptureKeyboard = true;
		ImGui::GetIO().WantCaptureMouse = true;*/
#endif

		// create game director
		m_gameDirector = GameDirector::CreateEditorGameDirectorPointer(m_renderSubsystem);

		m_timer.start();
		m_lastTime = static_cast<float>(m_timer.elapsed()) / 1000.0f;

		m_resizing = false;

		emit sigInitFinished();
	}

	void VulkanWindow::resizeEvent(QResizeEvent* event)
	{
		__super::resizeEvent(event);
		m_resizing = true;

		if (!m_initialized)
		{
			Initialize();
			requestUpdate();
			m_initialized = true;
			m_resizing = false;
		}
	}

	void VulkanWindow::keyPressEvent(QKeyEvent* event)
	{
		// 終了
		if (event->key() == Qt::Key_Escape)
		{
			emit sigCloseWindow();
		}
		int glfwKey = KeyConverter::Instance().QtToGlfw(event->key());
		auto key = m_keyActions.find(glfwKey);
		if (key != m_keyActions.end())
		{
			// 既に存在
			m_keyActions[glfwKey] = GLFW_REPEAT;
		}
		else
		{
			// 新規
			m_keyActions.insert_or_assign(glfwKey, GLFW_PRESS);
		}
		__super::keyPressEvent(event);
	}

	void VulkanWindow::keyReleaseEvent(QKeyEvent* event)
	{
		m_keyActions.insert_or_assign(KeyConverter::Instance().QtToGlfw(event->key()), GLFW_RELEASE);
		__super::keyReleaseEvent(event);
	}

	void VulkanWindow::mousePressEvent(QMouseEvent* event)
	{
		int glfwKey = KeyConverter::Instance().QtToGlfwMouse(event->button());
		auto key = m_mouseButtonActions.find(glfwKey);
		if (key != m_mouseButtonActions.end())
		{
			// 既に存在
			m_mouseButtonActions[glfwKey] = GLFW_REPEAT;
		}
		else
		{
			// 新規
			m_mouseButtonActions.insert_or_assign(glfwKey, GLFW_PRESS);
		}
		__super::mousePressEvent(event);
	}

	void VulkanWindow::mouseReleaseEvent(QMouseEvent* event)
	{
		int glfwKey = KeyConverter::Instance().QtToGlfwMouse(event->button());
		m_mouseButtonActions.insert_or_assign(glfwKey, GLFW_RELEASE);
		__super::mouseReleaseEvent(event);

		// オブジェクト選択
		if ((glfwKey == GLFW_MOUSE_BUTTON_LEFT) &&	// マウス左クリック
			(m_mouseButtonActions.size() == 1) &&	// 他のマウスキー押してない
			(m_keyActions.empty()) &&				// 他のキー押してない
			(!m_mouseDragging))						// Dragging状態じゃない
		{
			ThreadPool::Instance().enqueue([this] { this->objectSelect(); });
		}

		// 最後のマウスキーになるとDragging状態解除
		if (m_mouseButtonActions.size() == 1)
		{
			m_mouseDragging = false;
		}
	}

	void VulkanWindow::mouseMoveEvent(QMouseEvent* event)
	{
		if (!m_mouseButtonActions.empty())
		{
			m_mouseDragging = true;
		}
		__super::mouseMoveEvent(event);
	}

	void VulkanWindow::OpenFile(std::string filePath)
	{
		// ファイル名
		std::string fileName = std::filesystem::path(filePath).filename().string();

		m_gameDirector->LoadMFWorld(fileName);
	}

	const QList<QAction*> VulkanWindow::GetToolBarActions()
	{
		QList<QAction*> actions;

		// Save
		QAction* saveAction = new QAction(tr("Save"), this);
		saveAction->setShortcut(QKeySequence::Save);
		saveAction->setStatusTip(tr("Save the current file"));
		connect(saveAction, &QAction::triggered, this, [this]()
			{
				const MObjectListConstPtr& objects = m_renderSubsystem->GetMObjectRegistry()->GetMObjectList();
				std::string savePath = AppInfo::g_mfWorldFolder + std::string("test.mfworld");
				m_gameDirector->SaveMFWorld(savePath, objects);
			});
		actions.append(saveAction);

		// Save As
		QAction* saveAsAction = new QAction(tr("Save As..."), this);
		saveAsAction->setShortcut(QKeySequence::SaveAs);
		// connect(saveAsAction, &QAction::triggered, this, &VulkanWindow::saveAsSlot);
		actions.append(saveAsAction);

		// Separator
		QAction* separator = new QAction(this);
		separator->setSeparator(true);
		actions.append(separator);

		// Load
		QAction* loadAction = new QAction(tr("Load"), this);
		loadAction->setShortcut(QKeySequence::Open);
		// connect(loadAction, &QAction::triggered, this, &VulkanWindow::loadSlot);
		actions.append(loadAction);

		return actions;
	}

	bool VulkanWindow::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::Type::MouseButtonRelease)
		{
			QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if ((mouseEvent->button() == Qt::MouseButton::LeftButton) && (m_resizing))
			{
				m_renderSubsystem->Resize(m_surface, glm::ivec2(width(), height()));
				requestUpdate();
				m_resizing = false;
			}
			else if (event->type() == QEvent::Type::WindowStateChange)
			{
				m_renderSubsystem->Resize(m_surface, glm::ivec2(width(), height()));
				requestUpdate();
				m_resizing = false;
				return true;
			}
		}
		return __super::eventFilter(watched, event);
	}

	bool VulkanWindow::event(QEvent* event)
	{
		if (event->type() == QEvent::Type::UpdateRequest)
		{
			updateRender();
			return true;
		}
		return __super::event(event);
	}

	bool VulkanWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
	{
		if (m_initialized)
		{
			MSG* msg = static_cast<MSG*>(message);
			//ImGui_ImplWin32_WndProcHandlerEx(msg->hwnd, msg->message, msg->wParam, msg->lParam, ImGui::GetIO());
		}
		return __super::nativeEvent(eventType, message, result);
	}

	void VulkanWindow::updateRender()
	{
		if (!m_resizing)
		{
			float currentTime = static_cast<float>(m_timer.elapsed()) / 1000.0f;
			float deltaTime = currentTime - m_lastTime;
			float dpr = static_cast<float>(this->devicePixelRatio());

			m_lastTime = currentTime;

			UpdateData updateData{};
			updateData.pause = false;
			updateData.deltaTime = deltaTime;
			updateData.screenSize = glm::vec2(static_cast<float>(width()) * dpr, static_cast<float>(height()) * dpr);

			QPoint globalPos = QCursor::pos();
			QPointF localPos = mapFromGlobal(globalPos);
			updateData.mousePos = glm::vec2(static_cast<float>(localPos.x()) * dpr, static_cast<float>(localPos.y()) * dpr);
			updateData.mouseButtonActions = m_mouseButtonActions;
			updateData.keyActions = m_keyActions;
			m_renderSubsystem->Update(updateData);

			updateMousePositionMonitor(updateData.mousePos);

			m_renderSubsystem->Render();
			//m_renderSubsystem->EditorRender();
		}
		requestUpdate();

		// 後片付け
		for (auto it = m_keyActions.begin(); it != m_keyActions.end();)
		{
			if (it->second == GLFW_RELEASE)
			{
				it = m_keyActions.erase(it);
			}
			else
			{
				it++;
			}
		}
		for (auto it = m_mouseButtonActions.begin(); it != m_mouseButtonActions.end();)
		{
			if (it->second == GLFW_RELEASE)
			{
				it = m_mouseButtonActions.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void VulkanWindow::objectSelect()
	{
		QPoint globalPos = QCursor::pos();
		QPointF localPos = mapFromGlobal(globalPos);
		m_renderSubsystem->ObjectSelect(static_cast<int32_t>(localPos.x()), static_cast<int32_t>(localPos.y()));
	}

	void VulkanWindow::updateMousePositionMonitor(const glm::vec2& pos)
	{
		const QString text = QString::asprintf("MousePos: %.2f %.2f", pos.x, pos.y);
		m_mousePositionMonitor->setText(text);
	}
}