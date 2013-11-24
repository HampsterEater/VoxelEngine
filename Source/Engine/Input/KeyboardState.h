// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_KEYBOARDSTATE_
#define _ENGINE_INPUT_KEYBOARDSTATE_

struct KeyboardKey
{
	enum Type
	{
		Escape = 0,

		CapsLock,
		NumLock,
		ScrollLock,
		PrintScreen,
		Pause,
		Insert,
		Home,
		Delete,
		End,

		PageUp, PageDown,
		
		Tab,
		LeftShift, RightShift,
		LeftCtrl,  RightCtrl,
		WinLeft,   WinRight,
		LeftAlt,   RightAlt,
		
		Space, 
		Enter,
		Backspace,

		Minus,
		Plus,
		Bar,
		LeftBrace,
		RightBrace,
		At,
		Colon,
		Tidle,
		Comma,
		Period,
		ForwardSlash,
		Backslash,

		Left, Right, Up, Down,

		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, 
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

		NumPadDivide,
		NumPadMultiply,
		NumPadSubtract,
		NumPadPlus,
		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		NumPadPeriod,
		NumPadEnter,
	};
};

class KeyboardState
{
public:
	virtual bool Is_Key_Down	(KeyboardKey::Type type) = 0;
	virtual bool Was_Key_Pressed(KeyboardKey::Type type) = 0;

};

#endif

