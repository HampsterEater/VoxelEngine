// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Platform\Win32\Win32_Platform.h"
#include "Generic\Helper\StringHelper.h"

#include "Generic\Threads\Thread.h"

#include "Engine\Engine\GameEngineConfig.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Textures\PNG\PNGPixmapFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"
#include "Engine\IO\Win32\Win32_FileStreamFactory.h"
#include "Engine\Audio\FMod\FMod_SoundFactory.h"

#include <windows.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <dbghelp.h>
#include <stdarg.h>
#include <Shlobj.h>

#include <cstdio>

#include "Engine\Platform\Win32\Win32_EntryPoint.h"

Win32_Platform::Win32_Platform()
{
}

float Win32_Platform::Get_Ticks()
{
	static LARGE_INTEGER freq;			
	static LARGE_INTEGER start;
	static int state = 0; // 0=not initialized, -1=not supported, 1=supported

	if (state == 0)
	{
		if (QueryPerformanceFrequency(&freq) == 0)
		{
			state = -1;
		}
		else
		{
			QueryPerformanceCounter(&start);
			state = 1;
		}
	}

	// Not supported, use tick count.
	if (state == -1)
	{
		return (float)GetTickCount();
	}
	else
	{
		LARGE_INTEGER tickCount;
		QueryPerformanceCounter(&tickCount);

		float f = (float)freq.QuadPart / 1000.0;
		return float(tickCount.QuadPart - start.QuadPart) / f;
	}	
}

void Win32_Platform::Crack_Path(const char* path, std::vector<std::string>& segments)
{
	StringHelper::Split(path, '/', segments);
}

std::string Win32_Platform::Join_Path(std::string a, std::string b)
{
	if (a[a.size() - 1] == '/' || a[a.size() - 1] == '\\' ||
		b[0]			== '/' || b[0]			  == '\\')
	{
		return a + b;
	}
	else
	{
		return a + "/" + b;
	}
}

std::string Win32_Platform::Extract_Directory(std::string a)
{
	int index = a.find_last_of("/\\");
	if (index == std::string::npos)
	{
		return a;
	}
	else
	{
		return a.substr(0, index);
	}
}

std::string Win32_Platform::Extract_Filename(std::string a)
{
	int index = a.find_last_of("/\\");
	if (index == std::string::npos)
	{
		return a;
	}
	else
	{
		return a.substr(index + 1);
	}
}

bool Win32_Platform::Is_File(const char* path)
{
	DWORD flags = GetFileAttributesA(path);

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return false;
	}

	return true;
}

bool Win32_Platform::Is_Directory(const char* path)
{
	DWORD flags = GetFileAttributesA(path);

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

bool Win32_Platform::Create_Directory(const char* path, bool recursive)
{
	if (recursive == true)
	{
		std::vector<std::string> cracked;
		std::string crack_path = "";

		Crack_Path(path, cracked);
		
		for (int i = 0; i < (int)cracked.size(); i++)
		{
			if (crack_path != "")
			{
				crack_path += "/";
			}
			crack_path += cracked.at(i);
			
			if (!Is_Directory(crack_path.c_str()))
			{
				bool result = Create_Directory(crack_path.c_str(), false);
				if (result == false)
				{
					return false;
				}
			}
		}

		return true;
	}
	else
	{
		int result = CreateDirectoryA(path, NULL);
		return (result != 0);
	}
}

std::string	Win32_Platform::Get_Working_Dir()
{
    char path[512];
    
	if (GetCurrentDirectoryA(512, path) == 0)
    {
        return "";
    }

    return path;
}

void Win32_Platform::Set_Working_Dir(const char* path)
{
	SetCurrentDirectoryA(path);
}

int Win32_Platform::Get_Stack_Trace(StackFrame* frames, int max_frames, void* platform_specific, int offset)
{	
	DBG_ASSERT(max_frames <= 512);	
	memset(frames, 0, sizeof(StackFrame) * max_frames);

	int frame_count = 0;

	if (platform_specific != NULL)
	{
		// If thread is not given, use current.
		HANDLE thread = GetCurrentThread();
		struct _EXCEPTION_POINTERS* exceptioninfo = reinterpret_cast<struct _EXCEPTION_POINTERS *>(platform_specific);

		// Capture context for this thread.
		CONTEXT context;
		ZeroMemory(&context, sizeof(CONTEXT));
	
		// 32bit has no function call to get the current
		// threads context, so its asm trickery time :3
		#ifdef PLATFORM_32BIT
		
			if (exceptioninfo != NULL)
			{
				memcpy(&context, exceptioninfo->ContextRecord, sizeof(CONTEXT));
			}
			else
			{
				context.ContextFlags = CONTEXT_CONTROL;	
				__asm
				{
				Label:
					mov [context.Ebp], ebp;
					mov [context.Esp], esp;
					mov eax, [Label];
					mov [context.Eip], eax;
				}
			}

		// 64bit does though, w00t.
		#else
	
			if (exceptioninfo != NULL)
			{
				memcpy(&context, exceptioninfo->ContextRecord, sizeof(CONTEXT));
			}
			else
			{
				RtlCaptureContext(&context);
			}

		#endif
			
		// Build the initial stack frame.
		STACKFRAME64 stackFrame;
		DWORD		 machineType;
		ZeroMemory(&stackFrame, sizeof(STACKFRAME64));
		
		#ifdef PLATFORM_X86
			machineType                 = IMAGE_FILE_MACHINE_I386;
			stackFrame.AddrPC.Offset    = context.Eip;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.Ebp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.Esp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#elif PLATFORM_AMD64
			machineType                 = IMAGE_FILE_MACHINE_AMD64;
			stackFrame.AddrPC.Offset    = context.Rip;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.Rsp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.Rsp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#elif PLATFORM_ITANIUM64
			machineType                 = IMAGE_FILE_MACHINE_IA64;
			stackFrame.AddrPC.Offset    = context.StIIP;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.IntSp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrBStore.Offset= context.RsBSP;
			stackFrame.AddrBStore.Mode  = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.IntSp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#else
			DBG_LOG("Platform does not support stack walking.");
			return 0;

		#endif

		EnterCriticalSection(&g_dbghelp_critical_section);
		
		// Stack trace!
		int trace_offset = 0;
		while (frame_count < max_frames)
		{
			if (!StackWalk64(machineType, GetCurrentProcess(), thread, &stackFrame, 
								machineType == IMAGE_FILE_MACHINE_I386 ? NULL : &context, 
								NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			{
				// Failure :(
				break;
			}

			if (++trace_offset <= offset)
			{
				continue;
			}
		
			if (stackFrame.AddrPC.Offset != 0)
			{
		#ifdef PLATFORM_64BIT
				frames[frame_count].Address = stackFrame.AddrPC.Offset;
		#else
				frames[frame_count].Address = stackFrame.AddrPC.Offset;
		#endif
				frame_count++;
			}
			else
			{
				// Reached the base of the stack.
				break;
			}
		}
		
		LeaveCriticalSection(&g_dbghelp_critical_section);
	}
	else
	{
		void* sys_frames[512];
		frame_count = CaptureStackBackTrace(1 + offset, max_frames, sys_frames, NULL);
		for (int i = 0; i < frame_count; i++)
		{
			frames[i].Address = (u64)sys_frames[i];
		}
	}

	return frame_count;
}

void Win32_Platform::Resolve_Stack_Frame_Info(StackFrame& frame)
{
	DWORD64 displacement = 0;

	char buffer[sizeof(SYMBOL_INFO) + StackFrame::MAX_STRING_SIZE * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol	= (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct	= sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen		= StackFrame::MAX_STRING_SIZE;

	sprintf_s(frame.Name, StackFrame::MAX_STRING_SIZE, "0x%.16llx",frame.Address);

	EnterCriticalSection(&g_dbghelp_critical_section);

	if (SymFromAddr(GetCurrentProcess(), frame.Address, &displacement, pSymbol) == TRUE)
	{
		strcpy_s(frame.Name, StackFrame::MAX_STRING_SIZE, pSymbol->Name);

		IMAGEHLP_LINE64 line;
		DWORD lineDisplacement;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		if (SymGetLineFromAddr64(GetCurrentProcess(), frame.Address, &lineDisplacement, &line))
		{
			frame.Line = line.LineNumber;
			strcpy_s(frame.File, StackFrame::MAX_STRING_SIZE, line.FileName);
		}
		else
		{
			frame.Line = 0;
			strcpy_s(frame.File, StackFrame::MAX_STRING_SIZE, "<unknown>");
		}
	}
	else
	{
		s32 err = GetLastError();
		strcpy_s(frame.File, StackFrame::MAX_STRING_SIZE, "<unknown>");
		frame.Line = 0;
	}
			
	LeaveCriticalSection(&g_dbghelp_critical_section);
}