// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Math\RectanglePacker.h"

RectanglePacker::RectanglePacker()
{
}

RectanglePacker::RectanglePacker(int width, int height)
	: m_width(width)
	, m_height(height)
	, m_offset_x(0)
	, m_offset_y(0)
	, m_max_row_height(0)
	, m_spacing(1)
{
}

bool RectanglePacker::Pack(Point size, Rect& output)
{
	// Can we just plop it on the next space on the current row?
	if (m_offset_x + size.X < m_width &&
		m_offset_y + size.Y < m_height)
	{
		output.X		= m_offset_x;
		output.Y		= m_offset_y;
		output.Width	= size.X;
		output.Height	= size.Y;
		
		m_offset_x += output.Width + m_spacing;

		if (output.Height > m_max_row_height)
		{
			m_max_row_height = output.Height;
		}

		return true;
	}

	// Ok, move to next row!
	m_offset_x = 0;
	m_offset_y += m_max_row_height + m_spacing;
	m_max_row_height = size.Y;

	if (m_offset_x + size.X < m_width &&
		m_offset_y + size.Y < m_height)
	{
		output.X		= m_offset_x;
		output.Y		= m_offset_y;
		output.Width	= size.X;
		output.Height	= size.Y;
				
		m_offset_x = output.Width + m_spacing;

		if (output.Height > m_max_row_height)
		{
			m_max_row_height = output.Height;
		}

		return true;
	}

	// Gay, no space :(
	return false;
}
