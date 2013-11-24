// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIFrame.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"

UIFrame::UIFrame()
{
}

UIFrame::UIFrame(AtlasHandle* atlas, std::string name_pattern)
{
	int hash_mark = name_pattern.find('#');
	DBG_ASSERT(hash_mark != std::string::npos, "Could not find replaceable hash mark in name pattern '%s'.", name_pattern.c_str());

	std::string split_left  = name_pattern.substr(0, hash_mark);
	std::string split_right = name_pattern.substr(hash_mark + 1);

	Atlas* at = atlas->Get();
	for (int i = 0; i < 9; i++)
	{
		std::string name = split_left + StringHelper::To_String(i) + split_right;
		m_frames[i] = at->Get_Frame(name.c_str());
		DBG_ASSERT(m_frames[i] != NULL, "Could not find expected atlas frame '%s'.", name.c_str());
	}
}

void UIFrame::Draw_Frame(const FrameTime& time, UIManager* manager, AtlasRenderer& renderer, Rect rect)
{
	// Center
	renderer.Draw_Frame(time, m_frames[4], Rect(rect.X, rect.Y, rect.Width, rect.Height));
	
	// Top-Left
	renderer.Draw_Frame(time, m_frames[0], Point(rect.X, rect.Y));
	// Top-Middle
	renderer.Draw_Frame(time, m_frames[1], Rect(rect.X + m_frames[0]->Rectangle.Width, rect.Y, rect.Width - m_frames[0]->Rectangle.Width - m_frames[2]->Rectangle.Width, m_frames[1]->Rectangle.Height));
	// Top-Right
	renderer.Draw_Frame(time, m_frames[2], Point(rect.X + rect.Width - m_frames[2]->Rectangle.Width, rect.Y));

	// Left
	renderer.Draw_Frame(time, m_frames[3], Rect(rect.X, rect.Y + m_frames[3]->Rectangle.Height, m_frames[3]->Rectangle.Width, rect.Height - m_frames[0]->Rectangle.Height - m_frames[6]->Rectangle.Height));

	// Right
	renderer.Draw_Frame(time, m_frames[5], Rect(rect.X + rect.Width - m_frames[5]->Rectangle.Width, rect.Y + m_frames[5]->Rectangle.Height, m_frames[5]->Rectangle.Width, rect.Height - m_frames[2]->Rectangle.Height - m_frames[8]->Rectangle.Height));
	
	// Bottom-Left
	renderer.Draw_Frame(time, m_frames[6], Point(rect.X, rect.Y + rect.Height - m_frames[6]->Rectangle.Height));
	// Bottom-Middle
	renderer.Draw_Frame(time, m_frames[7], Rect(rect.X + m_frames[6]->Rectangle.Width, rect.Y + rect.Height - m_frames[7]->Rectangle.Height, rect.Width - m_frames[6]->Rectangle.Width - m_frames[8]->Rectangle.Width, m_frames[7]->Rectangle.Height));
	// Bottom-Right
	renderer.Draw_Frame(time, m_frames[8], Point(rect.X + rect.Width - m_frames[8]->Rectangle.Width, rect.Y + rect.Height - m_frames[8]->Rectangle.Height));
}
