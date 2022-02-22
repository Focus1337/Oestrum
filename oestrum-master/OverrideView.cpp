#include "hooks.h"
#include "global.h"
#include "sdk.h"
#include "GrenadePrediction.h"

CSettings* g_Settings = new CSettings();
struct PlayerSettings g_PlayerSettings[MAX_PLAYERS];

void __fastcall Hooks::OverrideView(void* _this, void* _edx, CViewSetup* setup)
{
	if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
	{
		if (Global::LocalPlayer && !Global::LocalPlayer->IsScoped() && Global::LocalPlayer->GetHealth() > 0)
		{
			setup->fov += g_Settings->misc_FOV;
		}
		if (Global::LocalPlayer)
		{
			grenade_prediction::Get().View();
		}
	}

	clientmodeVMT->GetOriginalMethod<OverrideViewFn>(18)(_this, setup);
}

float __stdcall GGetViewModelFOV()
{
	float fov = g_pClientModeHook->GetMethod<oGetViewModelFOV>(35)();

	if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		if (Global::LocalPlayer)
		{
				fov += g_Settings->misc_Viewmodel;
		}
	}
	return fov;
}