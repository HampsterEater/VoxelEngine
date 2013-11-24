// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Input\Win32\Win32_MouseState.h"
#include "Engine\Display\Win32\Win32_Display.h"

Win32_MouseState::Win32_MouseState()
{
	memset(m_button_previous_state, 0, sizeof(bool) * 3);
	memset(m_button_state, 0, sizeof(bool) * 3);
}

void Win32_MouseState::Tick(const FrameTime& time)
{
	for (int i = 0; i < 3; i++)
	{
		m_button_previous_state[i] = m_button_state[i];
	}
	m_button_state[(int)MouseButton::Left]		= (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	m_button_state[(int)MouseButton::Right]		= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	m_button_state[(int)MouseButton::Middle]	= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0; 
}

bool Win32_MouseState::Is_Button_Down(MouseButton::Type type)
{
	return m_button_state[(int)type];
}

bool Win32_MouseState::Was_Button_Clicked(MouseButton::Type type)
{
	return !m_button_state[(int)type] && m_button_previous_state[(int)type];
}

void Win32_MouseState::Set_Position(Point position)
{
	Win32_Display* display = static_cast<Win32_Display*>(Display::Get());
	HWND window_handle = display->Get_Window_Handle();

	POINT point;
	point.x = (int)position.X;
	point.y = (int)position.Y;

	ClientToScreen(window_handle, &point);
	SetCursorPos(point.x, point.y);
}

Point Win32_MouseState::Get_Position()
{
	Win32_Display* display = static_cast<Win32_Display*>(Display::Get());
	HWND window_handle = display->Get_Window_Handle();

	POINT point;
	GetCursorPos(&point);
	ScreenToClient(window_handle, &point);

	return Point(point.x, point.y);
}