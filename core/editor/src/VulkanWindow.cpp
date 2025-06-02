// Copyright (c) 2025 kong9812
#include "VulkanWindow.h"
#include "AppInfo.h"
#include "VK_Validation.h"

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
		m_timer()
	{
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
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hwnd = reinterpret_cast<HWND>(winId());
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		VK_VALIDATION(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif

		// create render subsystem
		m_renderSubsystem = Render::CreateRenderSubsystemPointer();
		m_renderSubsystem->Initialize(m_instance, m_surface);

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
		}
	}

	void VulkanWindow::keyPressEvent(QKeyEvent* event)
	{
		// 終了
		if (event->key() == Qt::Key_Escape)
		{
			emit closeWindow();
		}
		__super::keyPressEvent(event);
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
				m_initialized = false;
				m_resizing = false;
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
			m_renderSubsystem->Update(updateData);

			m_renderSubsystem->BeginRender();
			m_renderSubsystem->Compute();
			m_renderSubsystem->ShadowRender();
			m_renderSubsystem->MainRender();
			m_renderSubsystem->FinalCompositionRender();
			m_renderSubsystem->EndRender();
		}
		requestUpdate();
	}
}