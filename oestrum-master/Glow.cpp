#include "Glow.h"
#include "sdk.h"
#include "global.h"
#include "GameUtils.h"


void Glow::RenderGlow()
{
	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++)
	{
		auto &glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<CBaseEntity*>(glowObject.m_pEntity);

		if (glowObject.IsUnused())
			continue;

		if (!entity)
			continue;

		ClientClass* pClass = (ClientClass*)entity->GetClientClass();

		auto class_id = pClass->m_ClassID;
		Color color;

		float redent = g_Settings->color_entGlow[0] * 255;
		float greenent = g_Settings->color_entGlow[1] * 255;
		float blueent = g_Settings->color_entGlow[2] * 255;


		switch (class_id)
		{
		case ClassId_CCSPlayer:
		{
			bool is_enemy = entity->GetTeamNum() != Global::LocalPlayer->GetTeamNum();
			bool playerTeam = entity->GetTeamNum() == 2;

			if (!g_Settings->visuals_Glow || !entity->IsValidRenderable())
				continue;

			if (!is_enemy && g_Settings->visuals_EnemyOnly)
				continue;

			glowObject.m_nGlowStyle = 0;

			float red, green, blue;
			if (playerTeam)
			{
				red = g_Settings->color_Glow[0] * 255;
				green = g_Settings->color_Glow[1] * 255;
				blue = g_Settings->color_Glow[2] * 255;
			}
			else
			{
				red = g_Settings->color_Glow[0] * 255;
				green = g_Settings->color_Glow[1] * 255;
				blue = g_Settings->color_Glow[2] * 255;
			}

			color.SetColor(red, green, blue, g_Settings->visuals_Opacity);
			break;
		}
		case ClassId_CBaseAnimating:
		{
			if (!g_Settings->visuals_entGlow)
				continue;

			glowObject.m_nGlowStyle = 0;

			color.SetColor(redent, greenent, blueent);
			break;
		}
		case ClassId_CPlantedC4:
		{
			if (!g_Settings->visuals_entGlow)
				continue;

			glowObject.m_nGlowStyle = 0;

			color.SetColor(redent, greenent, blueent);
			break;
		}
		default:
		{
			if (entity->IsWeapon())
			{
				if (!g_Settings->visuals_entGlow)
					continue;

				glowObject.m_nGlowStyle = 0;

				color.SetColor(redent, greenent, blueent);
			}
		}
		}

		glowObject.m_flRed = color.r() / 255.0f;
		glowObject.m_flGreen = color.g() / 255.0f;
		glowObject.m_flBlue = color.b() / 255.0f;
		glowObject.m_flAlpha = color.a() / 255.0f;
		glowObject.m_bRenderWhenOccluded = true;
		glowObject.m_bRenderWhenUnoccluded = false;
	}
}