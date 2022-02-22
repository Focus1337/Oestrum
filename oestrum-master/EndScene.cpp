#include "sdk.h"
#include "hooks.h"
#include "global.h"
#include "ESP.h"
#include "ImGUI\imconfig.h"
#include "ImGUI\imgui.h"
#include "ImGUI\imgui_internal.h"
#include "ImGUI\stb_rect_pack.h"
#include "ImGUI\stb_textedit.h"
#include "ImGUI\stb_truetype.h"
#include "ImGUI\DX9\imgui_impl_dx9.h"
#include "Items.h"
#include "Config.h"
#include "GameUtils.h"
#include "Icons.h"
#include "Resolver.h"

#define dum() {ImGui::Dummy(ImVec2(10, 1)); ImGui::SameLine();}
#define dum2() {ImGui::Dummy(ImVec2(35, 1)); ImGui::SameLine();}
#define dum3() {ImGui::Dummy(ImVec2(80, 1)); ImGui::SameLine();}
#define dum4() {ImGui::Dummy(ImVec2(20, 1)); ImGui::SameLine();}
#define dum5() {ImGui::Dummy(ImVec2(70, 1)); ImGui::SameLine();}
#define spacefunc(name) {ImGui::NewLine(); ImGui::SameLine(40); ImGui::Text(name); ImGui::NewLine(); ImGui::SameLine(40); }
#define sssspppp() {ImGui::NewLine(); ImGui::SameLine(40); }

#define Checkbox(name, func) {dum(); ImGui::Checkbox(name, func);}
#define SliderFloater(name, name2, func, minval, maxval, format) {spacefunc(name);ImGui::SliderFloat(name2, func, minval, maxval, format);}
#define SliderFloat2(name, func, minval, maxval, format) {sssspppp();ImGui::SliderFloat(name, func, minval, maxval, format);}
#define SliderInt2(name, func, minval, maxval) {sssspppp();ImGui::SliderInt(name, func, minval, maxval);}
#define SliderInter(name, name2, func, minval, maxval) {spacefunc(name); ImGui::SliderInt(name2, func, minval, maxval);}
#define ComboBox(name, name2, func, item) {spacefunc(name);ImGui::Combo(name2, func, item, ARRAYSIZE(item));}
#define ComboBox2(name, func, item) {sssspppp();ImGui::Combo(name, func, item, ARRAYSIZE(item));}
#define BindKey(name, func) {style.Colors[ImGuiCol_Text] = ImVec4(0.412f, 0.412f, 0.412f, 1.f);ImGui::SameLine(170);dum5();ImGui::Hotkey(name, func, ImVec2(120, 20));style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);}
#define ColorPicker(name, func) {ImGui::SameLine(200);dum5();ImGui::MyColorEdit3(name, func, 1 << 7);}
#define BeginChilder(name, name2, x, y, border) {dum4();ImGui::Text(name);dum4();ImGui::BeginChild(name2, ImVec2(x, y), border);}

IDirect3DTexture9 *rage_image = nullptr;
IDirect3DTexture9 *visuals_image = nullptr;
IDirect3DTexture9 *misc_image = nullptr;
IDirect3DTexture9 *line_image = nullptr;
int iTab;

extern int userid;

PresentFn oPresent;
ImFont* Default;
ImFont* Tabs;
tReset oResetScene;

void SubTab(const char* name, int* v, int tab_id) 
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 oldtext = style.Colors[ImGuiCol_Text];
	ImVec4 oldbutton = style.Colors[ImGuiCol_Button];
	if (tab_id != 0)
	{
		ImGui::SameLine(153);
	}
	style.Colors[ImGuiCol_Button] = ImColor(26, 28, 33, 255);
	if (*v != tab_id)
	{
		style.Colors[ImGuiCol_Text] = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
	}
	if (ImGui::Button(name, ImVec2(153, 30)))
	{
		*v = tab_id;
	}
	style.Colors[ImGuiCol_Button] = oldbutton;
	style.Colors[ImGuiCol_Text] = oldtext;
}

void GUI_Init(IDirect3DDevice9* pDevice) // Setup for imgui
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplDX9_Init(Global::Window, pDevice);
	ImGuiStyle& style = ImGui::GetStyle();
	Default = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.0f);
	style.WindowRounding = 0.f;
	style.FramePadding = ImVec2(4, 1);
	style.WindowPadding = ImVec2(1, 1);
	style.ItemSpacing = ImVec2(5, 6);
	style.ScrollbarSize = 10.f;
	style.ScrollbarRounding = 0.f;
	style.GrabMinSize = 5.f;
	style.GrabRounding = 6.f;
	Global::Init = true;
}

void color()
{
	int MenuRed, MenuGreen, MenuBlue;

	MenuRed = g_Settings->color_Menu[0] * 255;
	MenuGreen = g_Settings->color_Menu[1] * 255;
	MenuBlue = g_Settings->color_Menu[2] * 255;

	ImColor mainColor = ImColor(MenuRed, MenuGreen, MenuBlue, 255);
	ImVec4 mainColorHovered = ImVec4(mainColor.Value.x + 0.1f, mainColor.Value.y + 0.1f, mainColor.Value.z + 0.1f, mainColor.Value.w);
	ImVec4 mainColorActive = ImVec4(mainColor.Value.x + 0.2f, mainColor.Value.y + 0.2f, mainColor.Value.z + 0.2f, mainColor.Value.w);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_CheckMark] = mainColor;
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_FrameBg] = ImColor(24, 24, 24, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = mainColorHovered;
	style.Colors[ImGuiCol_FrameBgActive] = mainColorActive;
	style.Colors[ImGuiCol_TitleBg] = mainColor;
	style.Colors[ImGuiCol_TitleBgCollapsed] = mainColor;
	style.Colors[ImGuiCol_TitleBgActive] = mainColor;
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = mainColor;
	style.Colors[ImGuiCol_SliderGrabActive] = mainColorHovered;
	style.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_Header] = mainColor;
	style.Colors[ImGuiCol_HeaderHovered] = mainColorHovered;
	style.Colors[ImGuiCol_HeaderActive] = mainColorActive;
	style.Colors[ImGuiCol_ColumnHovered] = mainColorHovered;
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
}

void RageTab()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::Columns(2, "yes", false);
	BeginChilder("Aimbot", "##aimbot", 306, 294, true);
	{
		static int subtab1 = 0;
		SubTab(XorStr("General"), &subtab1, 0);
		SubTab(XorStr("Hitscan"), &subtab1, 1);
		ImGui::Dummy(ImVec2(1, 3));
		if (subtab1 == 0)
		{
			Checkbox("  Enabled", &g_Settings->rage_Enabled);
			Checkbox("  On key", &g_Settings->rage_Onkey);
			BindKey("##ragekey", &g_Settings->rage_Key);
			ComboBox("Selection", "##selection", &g_Settings->rage_Selection, ItemSelection);
			Checkbox("  Automatic fire", &g_Settings->rage_AutoShot);
			Checkbox("  Automatic scope", &g_Settings->rage_AutoScope);
			Checkbox("  Automatic revolver", &g_Settings->rage_AutoRevolver);
			SliderFloater("Minimum damage", "##mindamage", &g_Settings->rage_Mindamage, 1, 100, "%.f%%");
			SliderFloater("Hit chance", "##hitchance", &g_Settings->rage_Hitchance, 0, 100, "%.f%%");
		}
		if (subtab1 == 1)
		{
			Checkbox("  Multi-point", &g_Settings->rage_MultipointEnable);
			if (g_Settings->rage_MultipointEnable)
			{
				SliderFloater("Head scale", "##multipointhead", &g_Settings->rage_MultipHead, 0.f, 1.f, "%.1f");
				SliderFloater("Body scale", "##multipointbody", &g_Settings->rage_MultipBody, 0.f, 1.f, "%.1f");
				SliderFloater("Active distance", "##distanceactive", &g_Settings->rage_MultipDistance, 0.f, 180.f, "%.f");
			}
			ComboBox("Hitscan", "##hitscan", &g_Settings->rage_Hitscan, ItemHitscan);
			if (g_Settings->rage_Hitscan == 1)
			{
				ImGui::Text("Hitboxes:");
				for (int i = 0; i < ARRAYSIZE(ItemHitboxes); ++i)
				{
					ImGui::Selectable(ItemHitboxes[i], &g_Settings->rage_Hitboxes[i]);
				}
			}
		}
	} ImGui::EndChild();
	
	BeginChilder("Fake lag", "##fakelag", 306, 125, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Enabled", &g_Settings->rage_LagsEnabled);
		SliderInter("Maximum", "##lagsmax", &g_Settings->rage_LagsMax, 1, 14);
		SliderInter("Minimum", "##lagsmin", &g_Settings->rage_LagsMin, 1, 14);
		SliderInter("In air", "##lagsair", &g_Settings->rage_LagsAir, 1, 14);
	} ImGui::EndChild();
	ImGui::NextColumn();
	
	BeginChilder("Other", "##other", 306, 199, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Remove recoil", &g_Settings->rage_Norecoil);
		Checkbox("  Anti-aim correction", &g_Settings->rage_Resolver);
		if (g_Settings->rage_Resolver)
		{
			Checkbox("  Clear old records", &g_Settings->rage_ResolverClearRecs);
		}
	//	Checkbox("  Animation test", &g_Settings->test_Animtest);
	//	SliderInter("Anim count", "##animcount", &g_Settings->test_Animcount, 1, 13);
	}ImGui::EndChild();
	
	BeginChilder("Anti-aimbot", "##antiaimbot", 306, 220, true);
	{
		static int subtab0 = 0;
		SubTab(XorStr("General"), &subtab0, 0);
		SubTab(XorStr("Angles"), &subtab0, 1);

		if (subtab0 == 0)
		{
			Checkbox("  Enabled", &g_Settings->aa_Enabled);
			Checkbox("  Disable while knife", &g_Settings->aa_DisableKnife);
			Checkbox("  Break lby", &g_Settings->aa_BreakLby);
			if (g_Settings->aa_BreakLby)
			{
				ComboBox("Break type", "##yawbreak", &g_Settings->aa_Breakertype, ItemAABreakType);
			}
			ComboBox("Yaw base", "##yawbase", &g_Settings->aa_YawBase, ItemAAYawBase);
			if (g_Settings->aa_YawBase == 2)
			{
				BindKey("##reversekey", &g_Settings->aa_Reversekey);
			}
			ComboBox("Fake yaw base", "##fyawbase", &g_Settings->aa_FYawBase, ItemAAFakeYawBase);
			if (g_Settings->aa_FYawBase == 2)
			{
				BindKey("##reversekey", &g_Settings->aa_Reversekey);
			}
		}
		if (subtab0 == 1)
		{
			ComboBox("Pitch", "##pitch", &g_Settings->aa_Pitch, ItemAAPitch);
			ComboBox("Stand yaw", "##realyawstand", &g_Settings->aa_RYawStand, ItemAAYawStand);
			if (g_Settings->aa_RYawStand > 0)
			{
				SliderFloat2("##standyawang", &g_Settings->aa_RYawAngStand, -180, 180, u8"%.f°");
			}
			ComboBox("Move yaw", "##realyawmove", &g_Settings->aa_RYawMove, ItemAAYawMove);
			if (g_Settings->aa_RYawMove > 0)
			{
				SliderFloat2("##moveyawang", &g_Settings->aa_RYawAngMove, -180, 180, u8"%.f°");
			}
			ComboBox("Fake yaw", "##fakeyaw", &g_Settings->aa_FYaw, ItemAAFakeYaw);
			if (g_Settings->aa_FYaw > 0)
			{
				SliderFloat2("##fakeyawang", &g_Settings->aa_FYawAng, -180, 180, u8"%.f°");
			}
		}

	} ImGui::EndChild();
}

void VisualsTab()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::Columns(2, "yes", false);
	BeginChilder("Player ESP", "##player", 306, 322, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Enabled", &g_Settings->visuals_Enabled);
		spacefunc("Opacity"); ImGui::SliderInt("##visualsalpha", &g_Settings->visuals_Opacity, 75, 255);
		Checkbox("  Enemy only", &g_Settings->visuals_EnemyOnly);
		Checkbox("  Bounding box", &g_Settings->visuals_BoundingBox);
		ColorPicker("##box", g_Settings->color_BoundingBox);
		Checkbox("  Health bar", &g_Settings->visuals_Health);
		Checkbox("  Name", &g_Settings->visuals_Name);
		ComboBox("Weapon", "##weapon", &g_Settings->visuals_Weapon, ItemWeaponMode);
		Checkbox("  Ammo", &g_Settings->visuals_Ammo);
		ColorPicker("##ammobar", g_Settings->color_Ammobar);
		Checkbox("  Resolver info", &g_Settings->visuals_ResolverInfo);
		ColorPicker("##resinfo", g_Settings->color_ResolverInfo);
		Checkbox("  Bot info", &g_Settings->visuals_BotInfo);
		ColorPicker("##botinfo", g_Settings->color_BotInfo);
		Checkbox("  Skeleton", &g_Settings->visuals_Bones);
		ColorPicker("##bones", g_Settings->color_Skeletons);
		Checkbox("  Glow", &g_Settings->visuals_Glow);
		ColorPicker("##glow", g_Settings->color_Glow);
		Checkbox("  Hitmarker", &g_Settings->visuals_Hitmarker);

	} ImGui::EndChild();
	BeginChilder("Colored models", "##models", 306, 97, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Player", &g_Settings->visuals_Chams);
		ColorPicker("##chams", g_Settings->color_Chams);
		Checkbox("  Player (behind wall)", &g_Settings->visuals_ChamsWall);
		ColorPicker("##chamswall", g_Settings->color_ChamsWall);
		ComboBox("Type", "##chamstype", &g_Settings->visuals_ChamsStyle, ItemChamsMode);

	}ImGui::EndChild();
	ImGui::NextColumn();
	BeginChilder("Other ESP", "##other", 306, 197, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Off screen", &g_Settings->visuals_Offscreen);
		ColorPicker("##offscreencolor", g_Settings->color_OffScreen);
		Checkbox("  Spread crosshair", &g_Settings->visuals_SpreadCircle);
		ColorPicker("##spreadcolor", g_Settings->color_SpreadCircle);
		Checkbox("  Bullet tracers", &g_Settings->visuals_BulletTracers);
		ColorPicker("##tracers", g_Settings->color_Bullettracer);
		Checkbox("  Damage indicator", &g_Settings->visuals_DamageIndicator);
		ColorPicker("##damageindicator", g_Settings->color_DamageIndicator);
		Checkbox("  Grenade trajectory", &g_Settings->visuals_GrenadePrediction);
		ImGui::Selectable("   Dropped weapons", &g_Settings->visuals_DroppedWeapon);
		ImGui::Selectable("   Grenades", &g_Settings->visuals_Nades);
		ImGui::Selectable("   Bomb", &g_Settings->visuals_PlantedBomb);
		Checkbox("   Name", &g_Settings->visuals_entName);
		ColorPicker("##filtername", g_Settings->color_entName);
		Checkbox("   Box", &g_Settings->visuals_entBox);
		ColorPicker("##filterbox", g_Settings->color_entBox);
		Checkbox("   Glow", &g_Settings->visuals_entGlow);
		ColorPicker("##filterglow", g_Settings->color_entGlow);


	} ImGui::EndChild();
	BeginChilder("Effects", "##effects", 306, 222, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		Checkbox("  Remove flash", &g_Settings->visuals_NoFlash);
		Checkbox("  Remove smoke", &g_Settings->visuals_NoSmoke);
		Checkbox("  Remove visual recoil", &g_Settings->visuals_VisNoRecoil);
		Checkbox("  Night mode", &g_Settings->visuals_Nightmode);
		ComboBox("Skybox", "##Skybox", &g_Settings->visuals_Skybox, ItemSkyboxmode);
		Checkbox("  Remove scope", &g_Settings->visuals_NoScope);
		Checkbox("  Remove blur", &g_Settings->visuals_NoBlur);
		Checkbox("  Disable post processing", &g_Settings->visuals_PostProc);
		ImGui::NewLine();
		ImGui::SameLine(40);
		ImGui::Text("Force third person");
		BindKey("##thirdperson", &g_Settings->misc_TPKey);
		ComboBox2("##angles", &g_Settings->misc_TPangles, ItemThirdpersonAngles);
		ComboBox("Fake angle ghost", "##ghost", &g_Settings->visuals_FakeAngleGhost, ItemGhost);
		ColorPicker("##ghostcolor", g_Settings->color_FakeAngleGhost);
		ImGui::Selectable("      Show real angles", &g_Settings->visuals_ShowReal);
		ImGui::Selectable("      Show fake angles", &g_Settings->visuals_ShowFake);
		ImGui::Selectable("      Show lby angles", &g_Settings->visuals_ShowLby);

	} ImGui::EndChild();
}

void MiscTab()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::Columns(2, "yes", false);
	BeginChilder("Miscellaneous", "##misc", 306, 444, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		SliderInter("Override FOV", "##fov", &g_Settings->misc_FOV, 0, 50);
		SliderInter("Override viewmodel", "##viewmodel", &g_Settings->misc_Viewmodel, 0, 50);
		Checkbox("  Bunny hop", &g_Settings->misc_Bhop);
		Checkbox("  Air strafe", &g_Settings->misc_AutoStrafe);
		Checkbox("  Pre-speed", &g_Settings->misc_Prespeed);
		if (g_Settings->misc_Prespeed)
		{
			BindKey("##speedkey", &g_Settings->misc_PrespeedKey);
			SliderInt2("##retrack", &g_Settings->misc_Retrack, 1, 8);
		}
		Checkbox("  Fakewalk", &g_Settings->misc_FakewalkEnabled);
		if (g_Settings->misc_FakewalkEnabled)
		{
			BindKey("##walkkey", &g_Settings->misc_FakewalkKey);
		}
		Checkbox("  Clan tag", &g_Settings->misc_Clantag);
		Checkbox("  Knife bot", &g_Settings->misc_KbEnabled);
		if (g_Settings->misc_KbEnabled)
		{
			Checkbox("  Crouch", &g_Settings->misc_KbCrouch);
			Checkbox("  Back only", &g_Settings->misc_KbBackOnly);
		}
		Checkbox("  Log dmg dealt", &g_Settings->misc_Logdmg);

	} ImGui::EndChild();
	ImGui::NextColumn();
	BeginChilder("GUI", "##gui", 306, 150, true);
	{
		ImGui::Dummy(ImVec2(1, 3));
		ImGui::Text("           Menu key");
		BindKey("##menukey", &g_Settings->misc_MenuKey);
		ImGui::Text("           Menu color");
		ColorPicker("##menucolor", g_Settings->color_Menu);
		Checkbox("  Anti-untrusted", &g_Settings->misc_AntiUntrust);
	} ImGui::EndChild();
	BeginChilder("Config", "##config", 306, 269, true);
	{
		style.Colors[ImGuiCol_Button] = ImColor(26, 28, 33, 255);

		static const char* Misc_Configs[] =
		{
			"Automatics",
			"Snipers",
			"Pistols",
			"Rifles"
		};

		ImGui::Dummy(ImVec2(1, 3));
		ImGui::SameLine(40);
		ImGui::Combo("##cfg", &g_Settings->misc_ConfigSelection, Misc_Configs, ARRAYSIZE(Misc_Configs));
		ImGui::NewLine();
		ImGui::SameLine(40);
		if (ImGui::Button("Save", ImVec2(201, 27)))
			ConSys->SaveConfig();
		ImGui::NewLine();
		ImGui::SameLine(40);
		if (ImGui::Button("Load", ImVec2(201, 27)))
			ConSys->LoadConfig();
		ImGui::NewLine();
		ImGui::SameLine(40);
		if (ImGui::Button("Reset", ImVec2(201, 27)))
			ConSys->Reset();

	} ImGui::EndChild();
}


HRESULT __stdcall Hooks::D3D9_EndScene(IDirect3DDevice9* pDevice)
{
	HRESULT result = d3d9VMT->GetOriginalMethod<EndSceneFn>(42)(pDevice);
	if (!Global::Init)
	{
		GUI_Init(pDevice);
	}
	else
	{
		if (g_pEngine->IsInGame() || g_pEngine->IsConnected())
			ImGui::GetIO().MouseDrawCursor = Global::Opened;
		else
			ImGui::GetIO().MouseDrawCursor = true;

		ImGui_ImplDX9_NewFrame();
		auto& style = ImGui::GetStyle();

		if (Global::Opened)
		{
			ImVec2 mainWindowPos; // menu pos
			int w, h;
			g_pEngine->GetScreenSize(w, h);

			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Appearing);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
			ImGui::Begin("##background", &Global::Opened, ImVec2(w, h), 0.5f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
			{

			}ImGui::End();

			ImGui::PushFont(Default);
			color();
			ImGui::SetNextWindowSize(ImVec2(700, 600));
			ImGui::Begin("Oestrum", &Global::Opened, ImVec2(700, 600), 0.98f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar);
			{
				mainWindowPos = ImGui::GetWindowPos();

				ImVec2 curPos = ImGui::GetCursorPos();
				ImVec2 curWindowPos = ImGui::GetWindowPos();
				curPos.x += curWindowPos.x;
				curPos.y += curWindowPos.y;

				int size;
				int y;
				g_pEngine->GetScreenSize(size, y);

				// categories
				if (rage_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(pDevice, &Rage_icon, sizeof(Rage_icon), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &rage_image);
				dum2(); if (ImGui::ImageButton(rage_image, ImVec2(90, 90))) iTab = 0;
				ImGui::SameLine();

				if (visuals_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(pDevice, &Visuals_icon, sizeof(Visuals_icon), 512, 282, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &visuals_image);
				ImGui::Dummy(ImVec2(85, 1)); ImGui::SameLine(); if (ImGui::ImageButton(visuals_image, ImVec2(115, 90))) iTab = 1;
				ImGui::SameLine();

				if (misc_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(pDevice, &Misc_icon, sizeof(Misc_icon), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &misc_image);
				dum3(); if (ImGui::ImageButton(misc_image, ImVec2(90, 90))) iTab = 2;
				ImGui::SameLine();

				auto userstr = "";

				if (userid == 0)
					userstr = "focus";
				else if (userid == 1)
					userstr = "krendix";
				else if (userid == 2)
					userstr = "tranti";
				else if (userid == 3)
					userstr = "pegasus";
				else if (userid == 4)
					userstr = "meow";
				else if (userid == 5)
					userstr = "speedy";
				else if (userid == 6)
					userstr = "spec";
				else if (userid == 7)
					userstr = "cracked!";

				ImGui::Dummy(ImVec2(13, 1)); ImGui::SameLine(); ImGui::Text("\n\n build: 41 \n last upd: 26.05.18 \n user: %s", userstr);

				style.Colors[ImGuiCol_Text] = iTab == 0 ? ImColor(255, 255, 255, 255) : ImColor(120, 120, 120, 255);
				ImGui::Dummy(ImVec2(58, 1)); ImGui::SameLine(); ImGui::Text("AIMBOT"); ImGui::SameLine();
				style.Colors[ImGuiCol_Text] = iTab == 1 ? ImColor(255, 255, 255, 255) : ImColor(120, 120, 120, 255);
				ImGui::Dummy(ImVec2(143, 1)); ImGui::SameLine(); ImGui::Text("VISUALS"); ImGui::SameLine();
				style.Colors[ImGuiCol_Text] = iTab == 2 ? ImColor(255, 255, 255, 255) : ImColor(120, 120, 120, 255);
				ImGui::Dummy(ImVec2(159, 1)); ImGui::SameLine(); ImGui::Text("MISC");

				if (line_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(pDevice, &Menu_line, sizeof(Menu_line), 700, 30, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &line_image);
				ImGui::Image(line_image, ImVec2(700, 5));

				style.Colors[ImGuiCol_Border] = ImColor(45, 44, 45, 255);
				style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				switch (iTab)
				{
				case 0: //rage
					RageTab();
					break;
				case 1: //visuals
					VisualsTab();
					break;
				case 2: //misc
					MiscTab();
					break;
				}
			}
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(mainWindowPos.x - 5, mainWindowPos.y - 5));
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.f);

			ImGui::SetNextWindowSize(ImVec2(710, 610));
			ImGui::Begin("##border", &Global::Opened, ImVec2(710, 610), 0.98f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_ShowBorders);
			{
				mainWindowPos = ImGui::GetWindowPos();
			} ImGui::End();
		}
		/*if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && g_Settings->test_Animtest)
		{
			ImVec2 PlistWindowPos;
			ImGui::PushFont(Default);
			color();
			ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Player animation", &g_Settings->test_Animtest, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar);
			{
				PlistWindowPos = ImGui::GetWindowPos();

				ImVec2 curPos = ImGui::GetCursorPos();
				ImVec2 curWindowPos = ImGui::GetWindowPos();
				curPos.x += curWindowPos.x;
				curPos.y += curWindowPos.y;
				style.Colors[ImGuiCol_Border] = ImColor(45, 44, 45, 255);
				style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
				{
					for (int i = 1; i <= g_pGlobals->maxClients; i++)
					{
						CBaseEntity *player = g_pEntitylist->GetClientEntity(i);
						if (!player)
							continue;
						if (player == Global::LocalPlayer)
							continue;
						//if (!player->IsAlive())
						//	continue;
						if (player->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
							continue;
						//if (player->GetHealth() <= 0)
						//	continue;
						//if (player->IsDormant())
						//	continue;

						for (int s = 0; s < g_Settings->test_Animcount; s++)
						{
							AnimationLayer layer = player->GetAnimOverlay(s);
							const int activity = player->GetSequenceActivity(layer.m_nSequence);
							float flcycle = layer.m_flCycle, flprevcycle = layer.m_flPrevCycle, flweight = layer.m_flWeight;

							if (player->IsAlive() && player->GetHealth() > 0 && !player->IsDormant())
							{
								ImGui::Text("ACT: %i", activity); ImGui::SameLine();
								ImGui::Text("CYCLE: %f", flcycle); ImGui::SameLine();
								ImGui::Text("PREV CYCLE: %f", flprevcycle); ImGui::SameLine();
								ImGui::Text("WEIGHT: %f", flweight); ImGui::NewLine();
							}
						}
					}
				}
			}
			ImGui::End();
		}*/

		ImGui::Render();

	}
	return result;
}

HRESULT __stdcall Hooks::hkdReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresParam)
{
	if (!Global::Init)
		return oResetScene(pDevice, pPresParam);

	ImGui_ImplDX9_InvalidateDeviceObjects();
	auto newr = oResetScene(pDevice, pPresParam);
	ImGui_ImplDX9_CreateDeviceObjects();

	return newr;
}
