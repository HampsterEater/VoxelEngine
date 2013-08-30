// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_
#define _ENGINE_RENDERER_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\AABB.h"
#include "Generic\Types\Frustum.h"

#include "Engine\Renderer\Textures\Texture.h"

class Display;
class Camera;

class Renderer : public Singleton<Renderer>
{
public:
	static Renderer* Create();

	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Draw(const FrameTime& time) = 0;

	// Display related settings.
	virtual bool Set_Display(Display* display) = 0;

	// Accessors.
	virtual Camera* Get_Active_Camera() = 0;

	// Visibility checking.
	virtual Frustum Get_Frustum() = 0;

	// Rendering! What we are all here for.
	virtual void	 Bind_Texture	(Texture* texture) = 0;
	virtual Texture* Create_Texture	(char* data, int width, int height, int pitch, TextureFormat::Type format) = 0;

	virtual void Push_Matrix() = 0;
	virtual void Pop_Matrix() = 0;
	virtual void Translate_World_Matrix(float x, float y, float z) = 0;
	virtual void Rotate_World_Matrix(float x, float y, float z) = 0;

	virtual void Render_Mesh(int id) = 0;
	virtual void Destroy_Mesh(int id) = 0;
	virtual int  Start_Mesh(int vertices, int triangles) = 0;
	virtual void End_Mesh(int id) = 0;
	virtual int  Add_Mesh_Vertex(int id, Vector3 position, Vector3 normal, float r, float g, float b, float a, float u, float v) = 0;
	virtual int  Add_Mesh_Triangle(int id, int vertex1, int vertex2, int vertex3) = 0;

	// Immediate rendering (mainly used for debugging).
	virtual void Draw_Wireframe_Cube(float w, float h, float d) = 0;
	virtual void Draw_Wireframe_Sphere(float r) = 0;
	virtual void Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2) = 0;

};

#endif

