// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIManager.h"
#include "Engine\UI\UIScene.h"
#include "Engine\UI\UITransition.h"

#include "Engine\Platform\Platform.h"

#include "Generic\Math\Math.h"

#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FontRenderer.h"

UIManager::UIManager()
	: m_scene_index(-1)
	, m_scene_change_time(0.0f)
	, m_transition(NULL)
	, m_previous_scene(NULL)
{
	m_font = FontFactory::Load("Data/Fonts/arial.xml", FontFlags::None);
	DBG_ASSERT(m_font != NULL);

	m_font_renderer = new FontRenderer(m_font, false);
}

UIManager::~UIManager()
{
	while (m_scene_index-- >= 0)
	{
		SAFE_DELETE(m_scenes[m_scene_index + 1]);
	}
	SAFE_DELETE(m_transition);
}

UIScene* UIManager::Get_Scene(int offset)
{
	return m_scenes[m_scene_index - (offset + 1)];
}

void UIManager::Tick(const FrameTime& time)
{
	UIScene* curr_scene = m_scenes[m_scene_index];
	UIScene* prev_scene = m_previous_scene;

	if (m_transition != NULL)
	{
		m_transition->Tick(time, this, prev_scene, curr_scene, m_scene_index);
		if (m_transition->Is_Complete())
		{
			SAFE_DELETE(m_previous_scene);
			SAFE_DELETE(m_transition);
		}
	}
	else if (curr_scene != NULL)
	{
		curr_scene->Tick(time, this, m_scene_index);
	}
}

void UIManager::Draw(const FrameTime& time)
{
	UIScene* curr_scene = m_scenes[m_scene_index];
	UIScene* prev_scene = m_previous_scene;

	if (m_transition != NULL)
	{
		m_transition->Draw(time, this, prev_scene, curr_scene, m_scene_index);
	}
	else if (curr_scene != NULL)
	{
		curr_scene->Draw(time, this, m_scene_index);
	}
	
	static float scale = 1.0f;
	static int scale_dir = 0;
	if (scale_dir == 0)
	{
		scale += 0.01;
		if (scale > 3.0f)
			scale_dir = 1;
	}
	else
	{
		scale -= 0.01;
		if (scale < 0.1f)
			scale_dir = 0;
	}

	float t = Platform::Get()->Get_Ticks();

	//m_font_renderer->Draw_String(time, " ", Point(0.0f, 0.0f), 32.0f * scale);
	//Point size = m_font_renderer->Calculate_String_Size(time, " !\"#$%&'()*+,-./\n0123456789:;<=>?@ABCDEFG\nHIJKLMNOPQRSTUVWXYZ[\]\n^_`abcdefghijklmnopqr\nstuvwxyz{|}~¦Çüéâäàåçê\nëèïîìÄÅÉæÆôöòûùÿÖ\nÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»¦¦\n¦¦¦ÁÂÀ©¦¦++¢¥++--+-+ãÃ\n++--¦-+¤ðÐÊËÈiÍÎÏ++¦_¦Ì¯ÓßÔÒõÕµþÞÚÛÙý\nÝ¯´­±=¾¶§÷¸°¨·¹³²¦", 32.0f * scale);
	
//	Renderer::Get()->Bind_Shader_Program();
	//Renderer::Get()->Draw_Quad(Rect(0, 0, 150.0f * scale, 150.0f * scale), Rect(0, 0, 1, 1));
	m_font_renderer->Draw_String(time, "It is a long established fact that a reader will be distracted by\n"
									   "the readable content of a page when looking at its layout. The\n"
									   "point of using Lorem Ipsum is that it has a more-or-less normal\n" 
									   "distribution of letters, as opposed to using 'Content here, content\n"
									   "here', making it look like readable English. Many desktop publishing\n"
									   "packages and web page", Rect(0.0f, 0.0f, 150.0f * scale, 150.0f * scale), TextAlignment::Right, TextAlignment::Bottom);
	
	float elapsed = Platform::Get()->Get_Ticks() - t;
	printf("Elapsed: %f\n", elapsed);

	//	m_font_renderer->Draw_String(time, "New File", Point(50.0f, 120.0f), 16.0f * scale);
}

void UIManager::Pop(UITransition* transition)
{
	DBG_ASSERT(m_scene_index >= 0);

	m_previous_scene = m_scenes[m_scene_index--];
	m_transition = transition;
	m_scene_change_time = Platform::Get()->Get_Ticks();

	if (transition != NULL)
	{
		DBG_LOG("Changed to scene '%s' with transition '%s'.", m_scenes[m_scene_index]->Get_Name(), transition->Get_Name());
	}
	else
	{
		DBG_LOG("Changed to scene '%s'.", m_scenes[m_scene_index]->Get_Name());	
	}
}

void UIManager::Push(UIScene* scene, UITransition* transition)
{
	DBG_ASSERT(m_scene_index < UI_MAX_SCENES - 1);

	m_previous_scene = m_scenes[m_scene_index];
	m_scenes[++m_scene_index] = scene;
	m_transition = transition;
	m_scene_change_time = Platform::Get()->Get_Ticks();

	if (transition != NULL)
	{
		DBG_LOG("Changed to scene '%s' with transition '%s'.", scene->Get_Name(), transition->Get_Name());
	}
	else
	{
		DBG_LOG("Changed to scene '%s'.", scene->Get_Name());	
	}
}
