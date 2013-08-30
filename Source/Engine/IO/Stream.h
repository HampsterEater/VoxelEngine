// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAM_
#define _ENGINE_STREAM_

struct StreamMode
{
	enum Type
	{
		Read		= 0x1, // Grant read access.
		Write		= 0x2, // Grant write access.
		Truncate	= 0x4, // Truncate file if it does exist, otherwise append.
		Create		= 0x8, // Create file if it dosen't already exist.
	};
};

class Stream
{
private:
	char* m_buffer;
	int   m_buffer_size;

public:
	
	// Constructor!
	Stream();
	~Stream();

	// Members that have to be overidden.
	virtual bool			IsEOF		() = 0;
	virtual unsigned int	Position	() = 0;
	virtual void			Seek		(unsigned int offset) = 0;
	virtual unsigned int	Length		() = 0;
	virtual void			Write		(const char* buffer, int offset, int length) = 0;
	virtual void			Read		(char* buffer, int offset, int length) = 0;
	virtual void			Close		() = 0;

	// Member methods.
	void			WriteLine			(const char* str);
	void			WriteString			(const char* str);
	void			WriteInt32			(int val);
	void			WriteUInt32			(unsigned int val);
	void			WriteInt16			(short val);
	void			WriteUInt16			(unsigned short val);
	void			WriteInt8			(char val);
	void			WriteUInt8			(unsigned char val);

	const char*		ReadLine			();
	const char*		ReadString			(int length);
	int				ReadInt32			();
	unsigned int	ReadUInt32			();
	short			ReadInt16			();
	unsigned short	ReadUInt16			();
	char			ReadInt8			();
	unsigned char	ReadUInt8			();

};

#endif

