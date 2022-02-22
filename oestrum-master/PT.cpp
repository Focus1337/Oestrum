#include "hooks.h"
#include "GameUtils.h"
#include "Draw.h"
#include "ESP.h"
#include "Misc.h"
#include "global.h"
#include "SpoofedConvar.h"
#include "GrenadePrediction.h"
#include "global.h"
#include "Hitmarker.h"
#include "ICvar.h"
#include "Resolver.h"

bool first_frame_passed = false;
std::string sPanel = XorStr("FocusOverlayPanel");

void __fastcall Hooks::PaintTraverse(void* ecx/*thisptr*/, void* edx, unsigned int vgui_panel, bool force_repaint, bool allow_force) // cl
{
	if (g_Settings->visuals_Enabled && g_Settings->visuals_NoScope && strcmp("HudZoom", g_pPanel->GetName(vgui_panel)) == 0)
		return;

	panelVMT->GetOriginalMethod<PaintTraverseFn>(41)(ecx, vgui_panel, force_repaint, allow_force);

	static bool bSpoofed = false;


	if (g_Settings->misc_TPKey > 0 && !bSpoofed)
	{
		ConVar* svcheats = g_pCvar->FindVar("sv_cheats");
		SpoofedConvar* svcheatsspoof = new SpoofedConvar(svcheats);
		svcheatsspoof->SetInt(1);
		bSpoofed = true;
	}

	const char* pszPanelName = g_pPanel->GetName(vgui_panel);


	if (!strstr(pszPanelName, sPanel.data()))
		return;

	static auto doOnce = false;
	if (!doOnce)
	{
		g_pEngine->ExecuteClientCmd("clear");
		g_pCvar->ConsoleColorPrintf(Color(37, 195, 219, 255), XorStr("Oestrum injected! \n"));
		doOnce = true;
	}

	int cur_height, cur_width; g_pEngine->GetScreenSize(cur_width, cur_height);

	if (!first_frame_passed || cur_width != Global::Screen.width || cur_height != Global::Screen.height)
	{
		first_frame_passed = true;
		g_Draw.Init();

		g_pEngine->GetScreenSize(cur_width, cur_height);
		Global::Screen.height = cur_height;
		Global::Screen.width = cur_width;
	}

	if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && Global::LocalPlayer)
	{
		if (g_Settings->visuals_DamageIndicator)
		{
			Global::CurrTime = g_pGlobals->interval_per_tick * (Global::LocalPlayer->GetTickBase() + 1);

			if (!Global::DamageHit.empty())
			{
				for (auto it = Global::DamageHit.begin(); it != Global::DamageHit.end();) {
					if (Global::CurrTime > it->ExpireTime) {
						it = Global::DamageHit.erase(it);
						continue;
					}
					it->Draw();
					++it;
				}
			}
		}

		if (g_Settings->visuals_NoScope)
		{
			g_ESP->DrawScope(Global::LocalPlayer);
		}

		g_ESP->Run();

		if (Global::LocalPlayer->IsAlive() || Global::LocalPlayer->GetHealth() >= 1)
		{
			grenade_prediction::Get().Paint();
		}

		// draw w/out net graph
		if (Global::Opened)
		{
			g_Draw.Box(0, 0, 0, 0, 255, 255, 255, 255);
		}

		CHitmarker::Get().draw();
	}
}