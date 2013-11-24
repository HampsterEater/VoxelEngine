// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_WIN32_DISPLAY_
#define _ENGINE_WIN32_DISPLAY_

#include "Engine\Display\Display.h"

#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLu.h>
#include <gl\glext.h>

#define KEY_COUNT 256

class Win32_Display : public Display
{
private:
	const char* m_title;
	int			m_width;
	int			m_height;
	bool		m_fullscreen;
	bool		m_active;

	LPCWSTR		m_class_name;
	HDC			m_device_context;
	HGLRC		m_render_context;
	HWND		m_window_handle;
	HINSTANCE	m_instance_handle;

	void Setup_Window();
	void Destroy_Window();

public:

	// Constructors.
	Win32_Display(const char* title, int width, int height, bool fullscreen);
	~Win32_Display();

	// Win32 specific.
	HWND Get_Window_Handle();
	
	// Base functions.	
	void Tick(const FrameTime& time);
	void Swap_Buffers();

	// Event handling.
	static LRESULT CALLBACK Static_Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	// Properties
	const char* Get_Title();
	int Get_Width();
	int Get_Height();
	int Get_Fullscreen();

	// Modifiers
	void Set_Title(const char* title);
	void Resize(int width, int height, bool fullscreen);
	void Toggle_Fullscreen();

	// Input.
	bool  Is_Active();

};

#endif

