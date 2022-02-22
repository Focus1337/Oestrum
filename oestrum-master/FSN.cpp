#include "hooks.h"
#include "global.h"
#include "xor.h"
#include <intrin.h>
#include "SpoofedConvar.h"
#include "Math.h"
#include "Items.h"
#include "Resolver.h"
#include "Aimbot.h"

ConVar* r_DrawSpecificStaticProp;
ConVar* developer;
ConVar* con_filter_text;
ConVar* con_filter_text_out;
ConVar* con_filter_enable;

std::vector<const char*> smoke_materials = {
	"particle/beam_smoke_01",
	"particle/particle_smokegrenade",
	"particle/particle_smokegrenade1",
	"particle/particle_smokegrenade2",
	"particle/particle_smokegrenade3",
	"particle/particle_smokegrenade_sc",
	"particle/smoke1/smoke1",
	"particle/smoke1/smoke1_ash",
	"particle/smoke1/smoke1_nearcull",
	"particle/smoke1/smoke1_nearcull2",
	"particle/smoke1/smoke1_snow",
	"particle/smokesprites_0001",
	"particle/smokestack",
	"particle/vistasmokev1/vistasmokev1",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_nearcull",
	"particle/vistasmokev1/vistasmokev1_nearcull_fog",
	"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev4_emods_nocull",
	"particle/vistasmokev1/vistasmokev4_nearcull",
	"particle/vistasmokev1/vistasmokev4_nocull"
};

void DrawBeam(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_pszModelName = "sprites/physbeam.vmt";
	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 1.0f;
	beamInfo.m_flWidth = 1.0f;
	beamInfo.m_flEndWidth = 1.0f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 0.0f;
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;

	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;

	Beam_t* myBeam = g_pViewRenderBeams->CreateBeamPoints(beamInfo);

	if (myBeam)
		g_pViewRenderBeams->DrawBeam(myBeam);
}

void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	static bool OldNightmode;
	static int OldSky;
	static bool OldEvents;

	if (!Global::LocalPlayer || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
	{
		clientVMT->GetOriginalMethod<FrameStageNotifyFn>(36)(curStage);
		OldNightmode = false;
		OldSky = 0;
		OldEvents = false;
		return;
	}

	if (OldEvents != g_Settings->misc_Logdmg)
	{
		if (!developer)
			developer = g_pCvar->FindVar("developer");
		developer->SetValue(1);

		if (!con_filter_text)
			con_filter_text = g_pCvar->FindVar("con_filter_text");
		con_filter_text->SetValue("hit ");

		if (!con_filter_text_out)
			con_filter_text_out = g_pCvar->FindVar("con_filter_text_out");
		con_filter_text_out->SetValue("");

		if (!con_filter_enable)
			con_filter_enable = g_pCvar->FindVar("con_filter_enable");
		con_filter_enable->SetValue(2);

		OldEvents = g_Settings->misc_Logdmg;
	}

	if (OldNightmode != g_Settings->visuals_Nightmode)
	{
		if (!r_DrawSpecificStaticProp)
			r_DrawSpecificStaticProp = g_pCvar->FindVar("r_DrawSpecificStaticProp");

		r_DrawSpecificStaticProp->SetValue(0);

		for (MaterialHandle_t i = g_pMaterialSystem->FirstMaterial(); i != g_pMaterialSystem->InvalidMaterial(); i = g_pMaterialSystem->NextMaterial(i))
		{
			IMaterial* pMaterial = g_pMaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
			{
				if (g_Settings->visuals_Nightmode)
				{
					if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
						pMaterial->ColorModulate(0.3f, 0.3f, 0.3f);
					else
						pMaterial->ColorModulate(0.05f, 0.05f, 0.05f);
				}
				else
					pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
			}

		}
		OldNightmode = g_Settings->visuals_Nightmode;
	}

	if (OldSky != g_Settings->visuals_Skybox)
	{
		auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(FindPatternIDA("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));

		int type = g_Settings->visuals_Skybox;

		if (type == 1)
			LoadNamedSky("sky_csgo_night02");
		else if (type == 2)
			LoadNamedSky("vertigo");
		else if (type == 3)
			LoadNamedSky("sky_cs15_daylight02_hdr");

		OldSky = g_Settings->visuals_Skybox;
	}

	static Vector oldViewPunch;
	static Vector oldAimPunch;

	Vector* view_punch = Global::LocalPlayer->GetViewPunchPtr();
	Vector* aim_punch = Global::LocalPlayer->GetPunchAnglePtr();

	CBaseEntity *pLocal = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (curStage == FRAME_RENDER_START && Global::LocalPlayer->GetHealth() > 0)
	{
		static bool enabledtp = false, check = false;

		if (GetAsyncKeyState(g_Settings->misc_TPKey))
		{
			if (!check)
				enabledtp = !enabledtp;
			check = true;
		}
		else
			check = false;

		if (enabledtp)
		{
			*reinterpret_cast<QAngle*>(reinterpret_cast<DWORD>(Global::LocalPlayer + 0x31C0 + 0x8)) = Global::AAAngle;
		}

		if (view_punch && aim_punch && g_Settings->visuals_VisNoRecoil)
		{
			oldViewPunch = *view_punch;
			oldAimPunch = *aim_punch;

			view_punch->Init();
			aim_punch->Init();
		}

		if (enabledtp && Global::LocalPlayer->IsAlive()) 
		{
			*(bool*)((DWORD)g_pInput + 0xA5) = true;
			*(float*)((DWORD)g_pInput + 0xA8 + 0x8) = 150; // Distance
		}
		else
		{
			*(bool*)((DWORD)g_pInput + 0xA5) = false;
			*(float*)((DWORD)g_pInput + 0xA8 + 0x8);
		}
	}

	if (curStage == FRAME_NET_UPDATE_START)
	{
		if (g_Settings->visuals_BulletTracers)
		{
			float Red, Green, Blue;

			Red = g_Settings->color_Bullettracer[0] * 255;
			Green = g_Settings->color_Bullettracer[1] * 255;
			Blue = g_Settings->color_Bullettracer[2] * 255;

			for (unsigned int i = 0; i < trace_logs.size(); i++) {

				auto *shooter = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(trace_logs[i].userid));

				if (!shooter) return;

				Color color;
				if (shooter->GetTeamNum() == 3)
					color = Color(Red, Green, Blue, 210);
				else
					color = Color(Red, Green, Blue, 210);

				DrawBeam(trace_logs[i].start, trace_logs[i].position, color);

				trace_logs.erase(trace_logs.begin() + i);
			}
		}


		for (auto material_name : smoke_materials) {
			IMaterial* mat = g_pMaterialSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Settings->visuals_NoSmoke ? true : false);
		}

		if (g_Settings->visuals_NoSmoke) {
			static int* smokecount = *(int**)(FindPatternIDA("client.dll", "A3 ? ? ? ? 57 8B CB") + 0x1);
			*smokecount = 0;
		}

	}

	auto& Resolver = CResolver::Get();

	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
		{
			for (int i = 1; i <= g_pGlobals->maxClients; i++)
			{
				auto entity = g_pEntitylist->GetClientEntity(i);
				if (!entity)
					continue;
				bool is_local_player = entity == Global::LocalPlayer;
				if (is_local_player)
					continue;
				if (entity == Global::LocalPlayer)
					continue;
				if (entity->GetTeamNum() == Global::LocalPlayer->GetTeamNum() && !is_local_player)
					continue;
				if (entity->GetHealth() <= 0)
					continue;
				if (entity->IsDormant())
					continue;

				player_info_t plInfo;

				if (g_pEngine->GetPlayerInfo(entity->GetIndex(), &plInfo))
				{
					if (plInfo.m_bIsFakePlayer)
						continue;

					if (g_Settings->rage_Resolver)
						Resolver.ResolvePlayers(entity);
				}

				//entity->UpdateClientSideAnimation();
			}
		}
	}

	if (curStage == FRAME_RENDER_START)
	{
		*(bool*)offs.bOverridePostProcessingDisable = g_Settings->visuals_PostProc;

		// pvs fix lol
		for (int i = 1; i <= g_pGlobals->maxClients; i++)
		{
			CBaseEntity *player = g_pEntitylist->GetClientEntity(i);

			if (!player)
				continue;

			if (player == Global::LocalPlayer)
				continue;

			*(int*)((uintptr_t)player + 0xA30) = g_pGlobals->framecount;
			*(int*)((uintptr_t)player + 0xA28) = 0;
		}

		if (g_Settings->visuals_NoFlash && Global::LocalPlayer)
			if (*Global::LocalPlayer->GetFlashDuration() > 0.f)
				*Global::LocalPlayer->GetFlashDuration() = 0.f;
	}
	// crash here
	clientVMT->GetOriginalMethod<FrameStageNotifyFn>(36)(curStage);

	if (curStage == FRAME_RENDER_START && Global::LocalPlayer && Global::LocalPlayer->GetHealth() > 0)
	{
		if (g_Settings->visuals_VisNoRecoil)
		{
			*aim_punch = oldAimPunch;
			*view_punch = oldViewPunch;
		}
	}

	if (curStage == FRAME_NET_UPDATE_END)
	{
		if (g_Settings->rage_Resolver)
			Resolver.UpdateLogs();
	}
}
