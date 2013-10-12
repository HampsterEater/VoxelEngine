// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_AUDIORENDERER_
#define _ENGINE_AUDIO_AUDIORENDERER_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\Matrix4.h"
#include "Generic\Types\AABB.h"
#include "Generic\Types\Frustum.h"
#include "Generic\Types\Color.h"
#include "Generic\Types\Rectangle.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\RenderTarget.h"
#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Material.h"

#include <vector>

class AudioRenderer : public Singleton<AudioRenderer>
{
public:
	static AudioRenderer* Create();

	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;

};

#endif

