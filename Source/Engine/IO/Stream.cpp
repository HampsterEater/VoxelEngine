// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\IO\Stream.h"

Stream::Stream()
	: m_buffer(NULL)
	, m_buffer_size(0)
{
}

Stream::~Stream()
{
	SAFE_DELETE(m_buffer);
}

void Stream::WriteLine(const char* str)
{
	Write(str, 0, strlen(str));
	Write("\r\n", 0, 2);
}

void Stream::WriteString(const char* str)
{
	Write(str, 0, strlen(str));
}

void Stream::WriteInt32(int val)
{
	Write(reinterpret_cast<char*>(&val), 0, sizeof(int));
}

void Stream::WriteUInt32(unsigned int val)
{
	Write(reinterpret_cast<char*>(&val), 0, sizeof(unsigned int));
}

void Stream::WriteInt16(short val)
{
	Write(reinterpret_cast<char*>(&val), 0, sizeof(short));
}

void Stream::WriteUInt16(unsigned short val)
{	
	Write(reinterpret_cast<char*>(&val), 0, sizeof(unsigned short));
}

void Stream::WriteInt8(char val)
{
	Write(reinterpret_cast<char*>(&val), 0, sizeof(char));
}

void Stream::WriteUInt8(unsigned char val)
{
	Write(reinterpret_cast<char*>(&val), 0, sizeof(unsigned char));
}

const char*	Stream::ReadLine()
{
	bool exit   = false;
	int  offset = 0;

	while (IsEOF() == false && exit == false)
	{
		char chr   = ReadInt8();
		bool store = true;

		if (chr == '\r')
		{
			if (IsEOF() == false)
			{
				char chr2 = ReadInt8();
				if (chr2 == '\n')
				{
					store = false;
				}
				else
				{
					Seek(Position() - 1);
				}
			}
		}
		else if (chr == '\n')
		{
			store = false;
		}

		if (store == true)
		{
			if (m_buffer_size <= offset + 1)
			{
				char* old_buffer		= m_buffer;
				int   old_buffer_size	= m_buffer_size;
				m_buffer				= new char[(m_buffer_size + 1) * 2];
				m_buffer_size			= (m_buffer_size + 1) * 2;

				memcpy(m_buffer, old_buffer, old_buffer_size);

				SAFE_DELETE(m_buffer);
			}

			m_buffer[offset] = chr;
			offset++;
		}
	}

	m_buffer[offset] = '\0';
	return m_buffer;
}

const char*	Stream::ReadString(int length)
{
	if (m_buffer == NULL || m_buffer_size <= length)
	{
		SAFE_DELETE(m_buffer);
		m_buffer = new char[length + 1];
		m_buffer_size = length + 1;
	}

	Read(m_buffer, 0, length);
	m_buffer[length + 1] = '\0';
	
	return m_buffer;
}

int	Stream::ReadInt32()
{
	char buffer[sizeof(int)];
	Read(buffer, 0, sizeof(int));
	int val = *reinterpret_cast<int*>(buffer);
	return val;
}

unsigned int Stream::ReadUInt32()
{
	char buffer[sizeof(unsigned int)];
	Read(buffer, 0, sizeof(unsigned int));
	unsigned int val = *reinterpret_cast<unsigned int*>(buffer);
	return val;
}

short Stream::ReadInt16()
{
	char buffer[sizeof(short)];
	Read(buffer, 0, sizeof(short));
	short val = *reinterpret_cast<short*>(buffer);
	return val;
}

unsigned short Stream::ReadUInt16()
{
	char buffer[sizeof(unsigned short)];
	Read(buffer, 0, sizeof(unsigned short));
	unsigned short val = *reinterpret_cast<unsigned short*>(buffer);
	return val;
}

char Stream::ReadInt8()
{
	char buffer[sizeof(char)];
	Read(buffer, 0, sizeof(char));
	char val = *reinterpret_cast<char*>(buffer);
	return val;
}

unsigned char Stream::ReadUInt8()
{
	char buffer[sizeof(unsigned char)];
	Read(buffer, 0, sizeof(unsigned char));
	unsigned char val = *reinterpret_cast<unsigned char*>(buffer);
	return val;
}

