// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <vector>
#include <Qt>
#include <GLFW/glfw3.h>

class KeyConverter
{
public:
	static KeyConverter& Instance()
	{
		static KeyConverter instance;
		return instance;
	}

	/// Keyobard
	// GLFW -> QT
	int GlfwToQt(int glfwKey) const
	{
		auto it = m_glfwToQt.find(glfwKey);
		if (it != m_glfwToQt.end()) return it->second;
		return -1;	// 未対応
	}
	// QT -> GLFW
	int QtToGlfw(int qtKey) const
	{
		auto it = m_qtToGlfw.find(qtKey);
		if (it != m_qtToGlfw.end()) return it->second;
		return -1;	// 未対応
	}

	/// Mouse
	// GLFW -> QT
	Qt::MouseButton GlfwToQtMouse(int glfwKey) const
	{
		auto it = m_glfwToQtMouse.find(glfwKey);
		if (it != m_glfwToQtMouse.end()) return it->second;
		return Qt::MouseButton::NoButton;	// 未対応
	}
	// QT -> GLFW
	int QtToGlfwMouse(Qt::MouseButton qtKey) const
	{
		auto it = m_qtToGlfwMouse.find(qtKey);
		if (it != m_qtToGlfwMouse.end()) return it->second;
		return -1;	// 未対応
	}

	// コピー・代入禁止
	KeyConverter(const KeyConverter&) = delete;
	KeyConverter& operator=(const KeyConverter&) = delete;
	KeyConverter(KeyConverter&&) = delete;
	KeyConverter& operator=(KeyConverter&) = delete;

private:
	// コンストラクタ
	KeyConverter()
	{
		{ // Keyboard
			const std::vector<std::pair<int, int>> keyboardKeyPairs =
			{
				// Printable keys
				{ GLFW_KEY_SPACE,          Qt::Key::Key_Space},           // Space bar
				{ GLFW_KEY_APOSTROPHE,     Qt::Key::Key_Apostrophe},      // '
				{ GLFW_KEY_COMMA,          Qt::Key::Key_Comma},           // ,
				{ GLFW_KEY_MINUS,          Qt::Key::Key_Minus},           // -
				{ GLFW_KEY_PERIOD,         Qt::Key::Key_Period},          // .
				{ GLFW_KEY_SLASH,          Qt::Key::Key_Slash},           // /
				{ GLFW_KEY_0,              Qt::Key::Key_0},               // 0
				{ GLFW_KEY_1,              Qt::Key::Key_1},               // 1
				{ GLFW_KEY_2,              Qt::Key::Key_2},               // 2
				{ GLFW_KEY_3,              Qt::Key::Key_3},
				{ GLFW_KEY_4,              Qt::Key::Key_4},
				{ GLFW_KEY_5,              Qt::Key::Key_5},
				{ GLFW_KEY_6,              Qt::Key::Key_6},
				{ GLFW_KEY_7,              Qt::Key::Key_7},
				{ GLFW_KEY_8,              Qt::Key::Key_8},
				{ GLFW_KEY_9,			   Qt::Key::Key_9},
				{ GLFW_KEY_SEMICOLON,      Qt::Key::Key_Semicolon },       // ;
				{ GLFW_KEY_EQUAL,          Qt::Key::Key_Equal },           // =
				{ GLFW_KEY_A,              Qt::Key::Key_A },
				{ GLFW_KEY_B,              Qt::Key::Key_B },
				{ GLFW_KEY_C,              Qt::Key::Key_C },
				{ GLFW_KEY_D,              Qt::Key::Key_D },
				{ GLFW_KEY_E,              Qt::Key::Key_E },
				{ GLFW_KEY_F,              Qt::Key::Key_F },
				{ GLFW_KEY_G,              Qt::Key::Key_G },
				{ GLFW_KEY_H,              Qt::Key::Key_H },
				{ GLFW_KEY_I,              Qt::Key::Key_I },
				{ GLFW_KEY_J,              Qt::Key::Key_J },
				{ GLFW_KEY_K,              Qt::Key::Key_K },
				{ GLFW_KEY_L,              Qt::Key::Key_L },
				{ GLFW_KEY_M,              Qt::Key::Key_M },
				{ GLFW_KEY_N,              Qt::Key::Key_N },
				{ GLFW_KEY_O,              Qt::Key::Key_O },
				{ GLFW_KEY_P,              Qt::Key::Key_P },
				{ GLFW_KEY_Q,              Qt::Key::Key_Q },
				{ GLFW_KEY_R,              Qt::Key::Key_R },
				{ GLFW_KEY_S,              Qt::Key::Key_S },
				{ GLFW_KEY_T,              Qt::Key::Key_T },
				{ GLFW_KEY_U,              Qt::Key::Key_U },
				{ GLFW_KEY_V,              Qt::Key::Key_V },
				{ GLFW_KEY_W,              Qt::Key::Key_W },
				{ GLFW_KEY_X,              Qt::Key::Key_X },
				{ GLFW_KEY_Y,              Qt::Key::Key_Y },
				{ GLFW_KEY_Z,              Qt::Key::Key_Z },
				{ GLFW_KEY_LEFT_BRACKET,   Qt::Key::Key_BracketLeft },     // [
				{ GLFW_KEY_BACKSLASH,      Qt::Key::Key_Backslash },
				{ GLFW_KEY_RIGHT_BRACKET,  Qt::Key::Key_BracketRight },
				{ GLFW_KEY_GRAVE_ACCENT,   Qt::Key::Key_QuoteLeft },       // `

				// Function keys
				{ GLFW_KEY_ESCAPE,         Qt::Key::Key_Escape },
				{ GLFW_KEY_ENTER,          Qt::Key::Key_Return },
				{ GLFW_KEY_TAB,            Qt::Key::Key_Tab },
				{ GLFW_KEY_BACKSPACE,      Qt::Key::Key_Backspace },
				{ GLFW_KEY_INSERT,         Qt::Key::Key_Insert },
				{ GLFW_KEY_DELETE,         Qt::Key::Key_Delete },
				{ GLFW_KEY_RIGHT,          Qt::Key::Key_Right },
				{ GLFW_KEY_LEFT,           Qt::Key::Key_Left },
				{ GLFW_KEY_DOWN,           Qt::Key::Key_Down },
				{ GLFW_KEY_UP,             Qt::Key::Key_Up },
				{ GLFW_KEY_PAGE_UP,        Qt::Key::Key_PageUp },
				{ GLFW_KEY_PAGE_DOWN,      Qt::Key::Key_PageDown },
				{ GLFW_KEY_HOME,           Qt::Key::Key_Home },
				{ GLFW_KEY_END,            Qt::Key::Key_End },
				{ GLFW_KEY_CAPS_LOCK,      Qt::Key::Key_CapsLock },
				{ GLFW_KEY_SCROLL_LOCK,    Qt::Key::Key_ScrollLock },
				{ GLFW_KEY_NUM_LOCK,       Qt::Key::Key_NumLock },
				{ GLFW_KEY_PRINT_SCREEN,   Qt::Key::Key_Print },
				{ GLFW_KEY_PAUSE,          Qt::Key::Key_Pause },

				// Function keys
				{ GLFW_KEY_F1,             Qt::Key::Key_F1 },
				{ GLFW_KEY_F2,             Qt::Key::Key_F2 },
				{ GLFW_KEY_F3,             Qt::Key::Key_F3 },
				{ GLFW_KEY_F4,             Qt::Key::Key_F4 },
				{ GLFW_KEY_F5,             Qt::Key::Key_F5 },
				{ GLFW_KEY_F6,             Qt::Key::Key_F6 },
				{ GLFW_KEY_F7,             Qt::Key::Key_F7 },
				{ GLFW_KEY_F8,             Qt::Key::Key_F8 },
				{ GLFW_KEY_F9,             Qt::Key::Key_F9 },
				{ GLFW_KEY_F10,            Qt::Key::Key_F10 },
				{ GLFW_KEY_F11,            Qt::Key::Key_F11 },
				{ GLFW_KEY_F12,            Qt::Key::Key_F12 },

				// Keypad
				{ GLFW_KEY_KP_0,           Qt::Key::Key_0 },
				{ GLFW_KEY_KP_1,           Qt::Key::Key_1 },
				{ GLFW_KEY_KP_2,           Qt::Key::Key_2 },
				{ GLFW_KEY_KP_3,           Qt::Key::Key_3 },
				{ GLFW_KEY_KP_4,           Qt::Key::Key_4 },
				{ GLFW_KEY_KP_5,           Qt::Key::Key_5 },
				{ GLFW_KEY_KP_6,           Qt::Key::Key_6 },
				{ GLFW_KEY_KP_7,           Qt::Key::Key_7 },
				{ GLFW_KEY_KP_8,           Qt::Key::Key_8 },
				{ GLFW_KEY_KP_9,           Qt::Key::Key_9 },
				{ GLFW_KEY_KP_DECIMAL,     Qt::Key::Key_Period },
				{ GLFW_KEY_KP_DIVIDE,      Qt::Key::Key_Slash },
				{ GLFW_KEY_KP_MULTIPLY,    Qt::Key::Key_Asterisk },
				{ GLFW_KEY_KP_SUBTRACT,    Qt::Key::Key_Minus },
				{ GLFW_KEY_KP_ADD,         Qt::Key::Key_Plus },
				{ GLFW_KEY_KP_ENTER,       Qt::Key::Key_Enter },
				{ GLFW_KEY_KP_EQUAL,       Qt::Key::Key_Equal },

				// Modifier keys
				{ GLFW_KEY_LEFT_SHIFT,     Qt::Key::Key_Shift },
				{ GLFW_KEY_LEFT_CONTROL,   Qt::Key::Key_Control },
				{ GLFW_KEY_LEFT_ALT,       Qt::Key::Key_Alt },
				{ GLFW_KEY_LEFT_SUPER,     Qt::Key::Key_Meta },
				//{ GLFW_KEY_RIGHT_SHIFT,    Qt::Key::Key_Shift },		// 右キーはツールで使わない
				//{ GLFW_KEY_RIGHT_CONTROL,  Qt::Key::Key_Control },
				//{ GLFW_KEY_RIGHT_ALT,      Qt::Key::Key_Alt },
				//{ GLFW_KEY_RIGHT_SUPER,    Qt::Key::Key_Meta },
				{ GLFW_KEY_MENU,           Qt::Key::Key_Menu }
			};
			for (const auto& pair : keyboardKeyPairs)
			{
				m_glfwToQt[pair.first] = pair.second;
				m_qtToGlfw[pair.second] = pair.first;
			}
		}
		{// Mouse
			const std::vector<std::pair<int, Qt::MouseButton>> mouseButtonPairs =
			{
				{GLFW_MOUSE_BUTTON_LEFT,     Qt::LeftButton},     // 左クリック
				{GLFW_MOUSE_BUTTON_RIGHT,    Qt::RightButton},    // 右クリック
				{GLFW_MOUSE_BUTTON_MIDDLE,   Qt::MiddleButton},   // ホイールクリック

				// サイドボタン（Qt::MouseButton には ExtraButton1, ExtraButton2 などがある）
				{GLFW_MOUSE_BUTTON_4,        Qt::ExtraButton1},   // サイドボタン 1
				{GLFW_MOUSE_BUTTON_5,        Qt::ExtraButton2}    // サイドボタン 2
			};
			for (const auto& pair : mouseButtonPairs)
			{
				m_glfwToQtMouse[pair.first] = pair.second;
				m_qtToGlfwMouse[pair.second] = pair.first;
			}
		}
	}

	// Keyboard
	std::unordered_map<int, int> m_glfwToQt;
	std::unordered_map<int, int> m_qtToGlfw;
	// Mouse
	std::unordered_map<int, Qt::MouseButton> m_glfwToQtMouse;
	std::unordered_map<Qt::MouseButton, int> m_qtToGlfwMouse;
};