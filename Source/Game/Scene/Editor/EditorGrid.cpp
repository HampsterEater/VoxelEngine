// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Editor\EditorGrid.h"

#include "Engine\Renderer\PrimitiveRenderer.h"

EditorGrid::EditorGrid()
	: m_grid_size(0.1f)
	, m_grid_expanse(140)
	, m_mesh_dirty(true)
	, m_mesh_id(-1)
{
}

void EditorGrid::Tick(const FrameTime& time)
{
	Renderer* renderer = Renderer::Get();
	
	if (m_mesh_dirty == true)
	{
		// Goodbye old mesh
		if (m_mesh_id >= 0)
		{
			renderer->Destroy_Mesh(m_mesh_id);
		}

		// Calculate and create new mesh!
		int total_cells = ((m_grid_expanse * 2) + 1) * ((m_grid_expanse * 2) + 1);
		m_mesh_id = renderer->Start_Mesh(MeshPrimitiveType::Line, total_cells * 4, total_cells * 4);

		float y = 0.0f;
		float line_size = 2.0f;

		for (int x = -m_grid_expanse; x <= m_grid_expanse; x++)
		{
			for (int z = -m_grid_expanse; z <= m_grid_expanse; z++)
			{
				float start_x = x * m_grid_size;
				float start_z = z * m_grid_size;
				float end_x   = start_x + m_grid_size;
				float end_z   = start_z + m_grid_size;

				float r = 0.1f;
				float g = 0.1f;
				float b = 0.1f;
				float a = 1.0f;

				int v1 = renderer->Add_Mesh_Vertex(m_mesh_id, Vector3(start_x, y, start_z), Vector3(0, 0, 0), r, g, b, a, 0.0f, 0.0f);
				int v2 = renderer->Add_Mesh_Vertex(m_mesh_id, Vector3(end_x, y, start_z), Vector3(0, 0, 0), r, g, b, a, 0.0f, 0.0f);
				int v3 = renderer->Add_Mesh_Vertex(m_mesh_id, Vector3(start_x, y, end_z), Vector3(0, 0, 0), r, g, b, a, 0.0f, 0.0f);
				int v4 = renderer->Add_Mesh_Vertex(m_mesh_id, Vector3(end_x, y, end_z), Vector3(0, 0, 0), r, g, b, a, 0.0f, 0.0f);
			
				// Left side of cell
				renderer->Add_Mesh_Primitive(m_mesh_id, v1, v2);
				renderer->Add_Mesh_Primitive(m_mesh_id, v1, v3);
				renderer->Add_Mesh_Primitive(m_mesh_id, v3, v4);
				renderer->Add_Mesh_Primitive(m_mesh_id, v2, v4);
			}
		}

		renderer->End_Mesh(m_mesh_id);

		m_mesh_dirty = false;
	}
}

void EditorGrid::Draw(const FrameTime& time, RenderPipeline* pipeline)
{
	Renderer* renderer = Renderer::Get();

	if (m_mesh_id >= 0)
	{
		renderer->Set_World_Matrix(Matrix4::Translate(m_position) * Matrix4::Rotate(m_rotation));
		pipeline->Update_Shader_Uniforms();
	
		renderer->Render_Mesh(m_mesh_id);
	}
}
