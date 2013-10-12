// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_RENDERTARGET_
#define _ENGINE_RENDERER_TEXTURES_RENDERTARGET_

class Texture;

struct RenderTargetBufferType
{
	enum Type
	{
		Color,
		Depth,
		Stencil
	};
};

struct OutputBufferType
{
	enum Type
	{
		BackBuffer,
		RenderTargetTexture0,
		RenderTargetTexture1,
		RenderTargetTexture2,
		RenderTargetTexture3,
		RenderTargetTexture4,
		RenderTargetTexture5,
		RenderTargetTexture6,
		RenderTargetTexture7,
		RenderTargetTexture8,
		RenderTargetTexture9,
	};
};

class RenderTarget
{
private:
	
protected:

	// Constructor!
	RenderTarget();

public:

	// Destructor!
	virtual ~RenderTarget();
	
	// Binding support.
	virtual void Bind_Texture(RenderTargetBufferType::Type type, const Texture* texture) = 0;
	virtual void Validate() = 0;

	// Create shader programs.
	static RenderTarget* Create();

};

#endif

