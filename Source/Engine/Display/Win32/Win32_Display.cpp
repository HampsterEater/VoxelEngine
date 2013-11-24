// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Display\Win32\Win32_Display.h"
#include "Engine\Engine\GameEngine.h"

Win32_Display::Win32_Display(const char* title, int width, int height, bool fullscreen)
	: m_width(width)
	, m_height(height)
	, m_title(title)
	, m_fullscreen(fullscreen)
	, m_class_name(NULL)
	, m_device_context(NULL)
	, m_render_context(NULL)
	, m_window_handle(NULL)
	, m_instance_handle(NULL)
	, m_active(false)
{	
	Setup_Window();
}
	
HWND Win32_Display::Get_Window_Handle()
{
	return m_window_handle;
}

Win32_Display::~Win32_Display()
{
	Destroy_Window();
}

const char* Win32_Display::Get_Title()
{
	return m_title;
}

int Win32_Display::Get_Width()
{
	return m_width;
}

int Win32_Display::Get_Height()
{
	return m_height;
}

int Win32_Display::Get_Fullscreen()
{
	return m_fullscreen;
}

bool Win32_Display::Is_Active()
{
	return m_active;
}

void Win32_Display::Set_Title(const char* title)
{
	m_title = title;
}

void Win32_Display::Resize(int width, int height, bool fullscreen)
{
	m_width = width;
	m_height = height;
	m_fullscreen = fullscreen;

	Destroy_Window();
	Setup_Window();
}

void Win32_Display::Toggle_Fullscreen()
{
	m_fullscreen = !m_fullscreen;

	Destroy_Window();
	Setup_Window();
}

void Win32_Display::Tick(const FrameTime& time)
{
	MSG message;

//#ifndef DEBUG_BUILD
//	do
//#endif
//	{
		// Pump message queue.
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				GameEngine::Get()->Stop();
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);   
			}
		}

		// Sleep if not active.
//#ifndef DEBUG_BUILD
//		if (m_active == false)
//		{
//			Sleep(100);
//		}
//#endif
//	} 
//#ifndef DEBUG_BUILD
//	while (m_active == false);
//#endif
}

LRESULT CALLBACK Win32_Display::Static_Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	Win32_Display* display = reinterpret_cast<Win32_Display*>(GetWindowLong(hwnd, GWLP_USERDATA));
	return display->Event_Handler(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK Win32_Display::Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_ACTIVATE:
		{
			m_active = (wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE);
			return 0;
		}
	case WM_SYSCOMMAND:
		{
			switch (wparam)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

	case WM_KEYDOWN:
		{
			return 0;
		}

	case WM_KEYUP:
		{
			return 0;
		}
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void Win32_Display::Setup_Window()
{
	WNDCLASSEX	window_class;
	RECT		window_rect;
	DWORD		dw_extented_style;
	DWORD		dw_style;
	int			color_depth;
	int			depth_depth;
	int			stencil_depth;
	int			screen_width;
	int			screen_height;

	screen_width		= GetSystemMetrics(SM_CXSCREEN);
	screen_height		= GetSystemMetrics(SM_CYSCREEN);

	window_rect.left	= (screen_width / 2) - (m_width / 2);
	window_rect.top		= (screen_height / 2) - (m_height / 2);
	window_rect.right	= window_rect.left + m_width;
	window_rect.bottom	= window_rect.top + m_height;

	color_depth			= 32;
	depth_depth			= 16;
	stencil_depth		= 0;

	dw_extented_style	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dw_style			= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	m_class_name		= L"Engine_Win32_Display";

	m_instance_handle = GetModuleHandle(NULL);
	DBG_ASSERT(m_instance_handle);

	// Convert title to widestring.
	int title_len = strlen(m_title);
	WCHAR* wchar_array = new WCHAR[title_len + 1];
	MultiByteToWideChar(0, 0, m_title, -1, wchar_array, title_len + 1);
	LPCWSTR wchar_title = wchar_array;

	// Setup window class.
	memset(&window_class, 0, sizeof(window_class));
	window_class.cbSize			= sizeof(WNDCLASSEX);
	window_class.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc	= &Win32_Display::Static_Event_Handler;
	window_class.cbClsExtra		= NULL;
	window_class.cbWndExtra		= NULL;
	window_class.hInstance		= m_instance_handle;
	window_class.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	window_class.hIconSm		= window_class.hIcon;
	window_class.hCursor		= LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	window_class.lpszMenuName	= NULL;
	window_class.lpszClassName	= m_class_name;

	// Register the window class.
	{
		ATOM ret = RegisterClassEx(&window_class);
		DBG_ASSERT(ret != 0);
	}

	// Fullscreen?
	if (m_fullscreen == true)
	{
		DEVMODE dmScreenSettings;                  
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));      

		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);      
		dmScreenSettings.dmPelsWidth    = m_width;            
		dmScreenSettings.dmPelsHeight   = m_height;          
		dmScreenSettings.dmBitsPerPel   = color_depth;            
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		
		int ret = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		DBG_ASSERT(ret == DISP_CHANGE_SUCCESSFUL);

		dw_extented_style	= WS_EX_APPWINDOW; 
		dw_style			= WS_POPUP;
	}
	
	// Make sure window rect is correct.
	{
		BOOL ret = AdjustWindowRectEx(&window_rect, dw_style, false, dw_extented_style);
		DBG_ASSERT(ret != 0);
	}

	// Create window.
	{
		m_window_handle = CreateWindowEx
			(
				dw_extented_style,
				m_class_name,
				wchar_title,
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dw_style,
				window_rect.left,
				window_rect.top,
				window_rect.right - window_rect.left,
				window_rect.bottom  - window_rect.top,
				NULL,
				NULL,
				m_instance_handle,
				NULL
			);
		DBG_ASSERT(m_window_handle != NULL);
	}

	// Attach extra data to window handle.
	{
		LONG_PTR ret = SetWindowLong(m_window_handle, GWLP_USERDATA, reinterpret_cast<LONG>(this));
		DBG_ASSERT(ret == 0);
	}

	// Setup pixel format.
	static PIXELFORMATDESCRIPTOR pixel_format =                
	{
		sizeof(PIXELFORMATDESCRIPTOR),                 
		1,                              
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,                       
		PFD_TYPE_RGBA,                      
		color_depth,                           // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                   // Color Bits Ignored
		0,									
		0,									// Shift Bit Ignored
		0,									// No Accumulation Buffer
		0, 0, 0, 0,                         // Accumulation Bits Ignored
		depth_depth,						// 16Bit Z-Buffer (Depth Buffer)
		stencil_depth,						// No Stencil Buffer
		0,									// No Auxiliary Buffer
		PFD_MAIN_PLANE,                     // Main Drawing Layer
		0,									
		0, 0, 0                             
	};

	// Get device context.
	{
		m_device_context = GetDC(m_window_handle);
		DBG_ASSERT(m_device_context != NULL);
	}
	
	// Find and select pixel format.
	{
		GLuint format = ChoosePixelFormat(m_device_context, &pixel_format);
		DBG_ASSERT(format != NULL);

		BOOL ret = SetPixelFormat(m_device_context, format, &pixel_format);
		DBG_ASSERT(ret == TRUE);
	}

	// Create rendering context.
	{
		m_render_context = wglCreateContext(m_device_context);
		DBG_ASSERT(m_render_context != NULL);
	}
	
	// Make rendering context current.
	{
		BOOL result = wglMakeCurrent(m_device_context, m_render_context);
		DBG_ASSERT(result == TRUE);
	}

	// Hide cursor.
	ShowCursor(false);

	// Show window.
	ShowWindow(m_window_handle, SW_SHOW);
	SetForegroundWindow(m_window_handle);
	SetFocus(m_window_handle);
}

void Win32_Display::Destroy_Window()
{
	// Destroy rendering context.
	if (m_render_context)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_render_context);
		m_render_context = NULL;
	}

	// Destroy device context.
	if (m_device_context)
	{
		ReleaseDC(m_window_handle, m_device_context);
		m_device_context = NULL;
	}

	// Re-enable mouse cursor.
	ShowCursor(true);

	// Leave fullscreen.
	ChangeDisplaySettings(NULL, 0);    

	// Destroy HWND.
	if (m_window_handle)
	{
		DestroyWindow(m_window_handle);
		m_window_handle = NULL;
	}

	// Unregister instance.
	if (m_instance_handle != NULL)
	{
		UnregisterClass(m_class_name, m_instance_handle);
		m_instance_handle = NULL;
	}
}

void Win32_Display::Swap_Buffers()
{
	glFlush();
	glFinish();
	SwapBuffers(m_device_context);
}
