// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Input\Win32\Win32_KeyboardState.h"
#include "Engine\Display\Win32\Win32_Display.h"

static const int g_keyboard_key_to_vk[256] = 
{
	VK_ESCAPE,

	VK_CAPITAL,
	VK_NUMLOCK,
	VK_SCROLL,
	VK_SNAPSHOT,
	VK_PAUSE,
	VK_PAUSE,
	VK_HOME,
	VK_DELETE,
	VK_END,

	VK_PRIOR, VK_NEXT,
		
	VK_TAB,
	VK_LSHIFT, VK_RSHIFT,
	VK_LCONTROL, VK_RCONTROL,
	VK_LWIN, VK_RWIN,
	VK_LMENU, VK_RMENU,
		
	VK_SPACE, 
	VK_RETURN,
	VK_BACK,

	VK_OEM_MINUS,
	VK_OEM_PLUS,
	VK_OEM_3,
	VK_OEM_4,
	VK_OEM_6,
	VK_OEM_7,
	VK_OEM_1,
	VK_OEM_8,
	VK_OEM_COMMA,
	VK_OEM_PERIOD,
	VK_OEM_2,
	VK_OEM_5,

	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,

	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,

	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,

	111,
	106,
	109,
	107,
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	110,
	VK_RETURN
};

Win32_KeyboardState::Win32_KeyboardState()
{
	memset(m_button_previous_state, 0, sizeof(bool) * 256);
	memset(m_button_state, 0, sizeof(bool) * 256);
}

void Win32_KeyboardState::Tick(const FrameTime& time)
{
	for (int i = 0; i < 256; i++)
	{
		m_button_previous_state[i] = m_button_state[i];
	}
	for (int i = 0; i < 256; i++)
	{
		m_button_state[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
	}
}

bool Win32_KeyboardState::Is_Key_Down(KeyboardKey::Type type)
{
	int resolved = g_keyboard_key_to_vk[(int)type];
	return m_button_state[resolved];
}

bool Win32_KeyboardState::Was_Key_Pressed(KeyboardKey::Type type)
{
	int resolved = g_keyboard_key_to_vk[(int)type];
	return !m_button_state[resolved] && m_button_previous_state[resolved];
}
