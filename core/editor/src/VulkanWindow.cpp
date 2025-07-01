// Copyright (c) 2025 kong9812
#include "VulkanWindow.h"
#include "AppInfo.h"
#include "VK_Validation.h"
#include "KeyConverter.h"
#include "ThreadPool.h"

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
		m_selectedObject(nullptr)
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
		m_renderSubsystem->Initialize(m_instance, m_surface);

		// Imgui
#ifdef _WIN32
		ASSERT(ImGui_ImplWin32_Init(hwnd), "Failed to init imgui win32.");
		ImGui::GetIO().WantCaptureKeyboard = true;
		ImGui::GetIO().WantCaptureMouse = true;
#endif

		// create game director
		m_gameDirector = GameDirector::CreateGameDirectorPointer(m_renderSubsystem);

		m_timer.start();
		m_lastTime = static_cast<float>(m_timer.elapsed()) / 1000.0f;

		m_resizing = false;
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
			emit closeWindow();
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

	bool VulkanWindow::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::Type::MouseButtonRelease)
		{
			QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if ((mouseEvent->button() == Qt::MouseButton::LeftButton) && (m_resizing))
			{
				m_renderSubsystem->Resize(m_surface, width(), height());
				requestUpdate();
				m_resizing = false;
			}
			else if (event->type() == QEvent::Type::WindowStateChange)
			{
				m_renderSubsystem->Resize(m_surface, width(), height());
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
			ImGui_ImplWin32_WndProcHandlerEx(msg->hwnd, msg->message, msg->wParam, msg->lParam, ImGui::GetIO());
		}
		return __super::nativeEvent(eventType, message, result);
	}

	void VulkanWindow::updateRender()
	{
		if (!m_resizing)
		{
			float currentTime = static_cast<float>(m_timer.elapsed()) / 1000.0f;
			float deltaTime = currentTime - m_lastTime;
			m_lastTime = currentTime;

			UpdateData updateData{};
			updateData.pause = false;
			updateData.deltaTime = deltaTime;
			updateData.screenSize = glm::vec2(static_cast<float>(width()), static_cast<float>(height()));

			QPoint globalPos = QCursor::pos();
			QPointF localPos = mapFromGlobal(globalPos);
			updateData.mousePos = glm::vec2(static_cast<float>(localPos.x()), static_cast<float>(localPos.y()));
			updateData.mouseButtonActions = m_mouseButtonActions;
			updateData.keyActions = m_keyActions;
			m_renderSubsystem->Update(updateData);

			m_renderSubsystem->BeginRender();
			m_renderSubsystem->FrustumCuilling();
			m_renderSubsystem->ShadowRender();
			m_renderSubsystem->MainRender();
			m_renderSubsystem->EditorRender();
			m_renderSubsystem->FinalCompositionRender();
			m_renderSubsystem->EndRender();
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
		Logger::Debug("[VulkanWindow] objectSelect!!");
		QPoint globalPos = QCursor::pos();
		QPointF localPos = mapFromGlobal(globalPos);
		m_renderSubsystem->ObjectSelect(static_cast<int32_t>(localPos.x()), static_cast<int32_t>(localPos.y()));
	}
}