#include "sdk.h"
#include "hooks.h"
#include "MaterialHelper.h"
#include "security.h"
#include "Math.h"
#include "global.h"
#include <dbghelp.h>
#include <tchar.h>
#include "Config.h"
#include "ICvar.h"
#include "Misc.h"

#pragma region HWID Check
#define Me (-1328438005)
#define krendix (1611472574)
#define tranti (-1144925152)
#define pegasus (1581980313)
#define meow (123456) // unknwn
#define speedy (-1963811377)
#define spec (-2065098553)

int userid;

UCHAR szFileSys[255], szVolNameBuff[255];
DWORD dwMFL, dwSysFlags;
DWORD dwSerial;
LPCTSTR szHD = "C:\\";
#pragma endregion

HINSTANCE hAppInstance;
CClient* g_pClient;
CClientEntityList* g_pEntitylist;
CEngine* g_pEngine;
CInput* g_pInput;
CModelInfo* g_pModelInfo;
CInputSystem* g_pInputSystem;
CPanel* g_pPanel;
CSurface* g_pSurface;
CEngineTrace* g_pEngineTrace;
CDebugOverlay* g_pDebugOverlay;
IPhysicsSurfaceProps* g_pPhysics;
CRenderView* g_pRenderView;
CClientModeShared* g_pClientMode;
CGlobalVarsBase* g_pGlobals;
CModelRender* g_pModelRender;
CGlowObjectManager* g_GlowObjManager;
CMaterialSystem* g_pMaterialSystem;
IMoveHelper* g_pMoveHelper;
CPrediction* g_pPrediction;
CGameMovement* g_pGameMovement;
IGameEventManager* g_pGameEventManager;
IEngineVGui* g_pEngineVGUI;
ICvar* g_pCvar;
CInterfaces Interfaces;
CCRC gCRC;
C_TEFireBullets* g_pFireBullet;
IViewRenderBeams* g_pViewRenderBeams;

VMT* panelVMT;
VMT* clientmodeVMT;
VMT* enginevguiVMT;
VMT* modelrenderVMT;
VMT* clientVMT;
VMT* d3d9VMT;
VMT* renderviewVMT;
VMT* materialsystemVMT;
CVMTHookManager* g_pD3D = nullptr;
CVMTHookManager* g_pClientModeHook = nullptr;

HMODULE h_ThisModule;

void Init()
{
	SetupOffsets();
	g_MaterialHelper = new CMaterialHelper();
	g_Event.RegisterSelf();
	srand(time(0));

	ConSys->CheckConfigs();
	ConSys->Handle();

	panelVMT = new VMT(g_pPanel);
	panelVMT->HookVM((void*)Hooks::PaintTraverse, 41);
	panelVMT->ApplyVMT();

	g_GlowObjManager = *(CGlowObjectManager**)(FindPatternIDA("client.dll", "0F 11 05 ? ? ? ? 83 C8 01 C7 05 ? ? ? ? 00 00 00 00") + 3);

	clientmodeVMT = new VMT(g_pClientMode);
	clientmodeVMT->HookVM((void*)Hooks::CreateMove, 24);
	clientmodeVMT->HookVM((void*)Hooks::OverrideView, 18);
	clientmodeVMT->ApplyVMT();

	g_pClientModeHook = new CVMTHookManager((PDWORD*)g_pClientMode);
	g_pClientModeHook->HookMethod((DWORD)&GGetViewModelFOV, 35);
	g_pClientModeHook->ReHook();

	renderviewVMT = new VMT(g_pRenderView);
	renderviewVMT->HookVM((void*)Hooks::scene_end, 9);
	renderviewVMT->ApplyVMT();

	modelrenderVMT = new VMT(g_pModelRender);
	modelrenderVMT->HookVM((void*)Hooks::DrawModelExecute, 21);
	modelrenderVMT->ApplyVMT();

	clientVMT = new VMT(g_pClient);
	clientVMT->HookVM((void*)Hooks::FrameStageNotify, 36);
	clientVMT->ApplyVMT();

	materialsystemVMT = new VMT(g_pMaterialSystem);
	materialsystemVMT->HookVM((IMaterial*)Hooks::FindMaterial, 84);
	materialsystemVMT->ApplyVMT();

	auto dwDevice = **(uint32_t**)(FindPatternIDA(XorStr("shaderapidx9.dll"), XorStr("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 1);

	d3d9VMT = new VMT((void*)dwDevice);
	d3d9VMT->HookVM((void*)Hooks::D3D9_EndScene, 42);
	d3d9VMT->ApplyVMT();

	g_pD3D = new CVMTHookManager(reinterpret_cast<DWORD**>(dwDevice));
	oResetScene = reinterpret_cast<tReset>(g_pD3D->HookMethod(reinterpret_cast<DWORD>(Hooks::hkdReset), 16));

	Hooks::g_pOldWindowProc = (WNDPROC)SetWindowLongPtr(Global::Window, GWLP_WNDPROC, (LONG_PTR)Hooks::WndProc);
}

void StartCheat()
{
	if (Interfaces.GetInterfaces() && g_pPanel && g_pClientMode)
	{
		GetVolumeInformation("C:\\", (LPTSTR)szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, (LPTSTR)szFileSys, 255);
		if (dwSerial == Me || dwSerial == krendix || dwSerial == tranti || dwSerial == pegasus || dwSerial == meow || dwSerial == speedy || dwSerial == spec)
		{
			player_info_t pInfo;
			auto focus_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:1:124854201"));
			auto focus_steam2 = strcmp(pInfo.m_szSteamID, ("STEAM_0:1:878331"));
			auto krendix_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:0:214691399"));
			auto tranti_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:0:219495118"));
			auto pegasus_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:1:44697121"));
			auto meow_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:0:88879257"));
			auto speedy_steam = strcmp(pInfo.m_szSteamID, ("STEAM_0:1:219396435"));
			auto spec_steam = strcmp(pInfo.m_szSteamID, ("STEAM_1:1:2172634"));

			if (!focus_steam || !focus_steam2 || !krendix_steam || !tranti_steam || !pegasus_steam || !meow_steam || !speedy_steam || !spec_steam)
			{
				MessageBox(0, "[ERROR] Wrong HWID!", "Oestrum", MB_OK | MB_ICONINFORMATION);
				FreeLibraryAndExitThread(hAppInstance, 0x0);
				return;
			}
			if (g_pEngine->GetEngineBuildNumber() != 13638)
			{
				MessageBox(NULL, "[ERROR] Oestrum not updated!", "Oestrum", 0);
				FreeLibraryAndExitThread(hAppInstance, 0x0);
				return;
			}

			if (dwSerial == Me || focus_steam || focus_steam2)
				userid = 0;
			else if (dwSerial == krendix || krendix_steam)
				userid = 1;
			else if (dwSerial == tranti || tranti_steam)
				userid = 2;
			else if (dwSerial == pegasus || pegasus_steam)
				userid = 3;
			else if (dwSerial == meow || meow_steam)
				userid = 4;
			else if (dwSerial == speedy || speedy_steam)
				userid = 5;
			else if (dwSerial == spec || spec_steam)
				userid = 6;
			else
				userid = 7;

			Sleep(500);
			Init();
		}
		else
		{
			MessageBox(0, "[ERROR] Wrong HWID!", "Oestrum", MB_OK | MB_ICONINFORMATION);
			FreeLibraryAndExitThread(hAppInstance, 0x0);
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE Instance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(Instance);
		Global::Window = FindWindowA(("Valve001"), 0);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartCheat, 0, 0, 0);
		break;
	}
	return true;
}

MsgFn oMsg;

void __cdecl Msg(char const* msg, ...)
{
	//DOES NOT CRASH
	if(!oMsg)
		oMsg = (MsgFn)GetProcAddress(GetModuleHandle(XorStr("tier0.dll")), XorStr("Msg"));

	char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf_s(buffer, msg, list);
	va_end(list);
	oMsg(buffer, list);
}