#include "sdk.h"
#include <winerror.h>
#pragma warning( disable : 4091)
#include <ShlObj.h>
#include <string>
#include <sstream>
#include "xor.h"
#include "Config.h"

#pragma region Disabling warnings

#pragma warning(disable: 4244)
#pragma warning(disable: 4800)
#pragma warning(disable: 4018)
#pragma warning(disable: 4715)

#pragma endregion

Config* ConSys = new Config();

inline bool Check(std::string File)
{
	struct stat buf;
	return (stat(File.c_str(), &buf) == 0);
}

bool Config::CheckConfigs()
{
	/*=====================================*/
	strcat(Path, "C:\\Oestrum");
	CreateDirectoryA(Path, nullptr);
	strcpy(Path2, Path);
	strcpy(Path3, Path);
	strcpy(Path4, Path);

	strcat(Path, "\\Automatics.ini");
	strcat(Path2, "\\Snipers.ini");
	strcat(Path3, "\\Pistols.ini");
	strcat(Path4, "\\Rifles.ini");

	return true;
}

char* Config::Handle()
{
	if (g_Settings->misc_ConfigSelection == 0)
	{
		ConSys->Snipers = false;
		ConSys->Automatics = true;
		ConSys->Pistols = false;
		ConSys->Rifles = false;
	}

	if (g_Settings->misc_ConfigSelection == 1)
	{
		ConSys->Snipers = true;
		ConSys->Automatics = false;
		ConSys->Pistols = false;
		ConSys->Rifles = false;
	}
	
	if (g_Settings->misc_ConfigSelection == 2)
	{
		ConSys->Snipers = false;
		ConSys->Automatics = false;
		ConSys->Pistols = true;
		ConSys->Rifles = false;
	}

	if (g_Settings->misc_ConfigSelection == 3)
	{
		ConSys->Snipers = false;
		ConSys->Automatics = false;
		ConSys->Pistols = false;
		ConSys->Rifles = true;
	}
	if (Automatics)
		return Path;
	if (Snipers)
		return Path2;
	if (Pistols)
		return Path3;
	if (Rifles)
		return Path4;
}

int Config::ReadInt(char* Cata, char* Name, int DefaultVal)
{
	int iResult;
	iResult = GetPrivateProfileIntA(Cata, Name, DefaultVal, this->Handle());
	return iResult;
}

float Config::ReadFloat(char* Cata, char* Name, float DefaultVal)
{
	char result[255];
	char cDefault[255];
	float fresult;
	sprintf(cDefault, "%f", DefaultVal);
	GetPrivateProfileStringA(Cata, Name, cDefault, result, 255, this->Handle());
	fresult = atof(result);
	return fresult;
}

char* Config::ReadString(char* Cata, char* Name, char* DefaultVal)
{
	auto result = new char[255];
	DWORD oProtection;
	VirtualProtect(reinterpret_cast<void*>(result), 4, PAGE_READWRITE, &oProtection);
	memset(result, 0x00, 255);
	VirtualProtect(reinterpret_cast<void*>(result), 4, oProtection, &oProtection);
	GetPrivateProfileStringA(Cata, Name, DefaultVal, result, 255, this->Handle());
	return result;
}

void Config::WriteFloat(char* Cata, char* Name, float SetVal)
{
	char buf[255];
	sprintf(buf, "%f", SetVal);
	WritePrivateProfileStringA(Cata, Name, buf, this->Handle());
}

void Config::WriteInt(char* Cata, char* Name, int SetVal)
{
	char buf[255];
	sprintf(buf, "%d", SetVal);
	WritePrivateProfileStringA(Cata, Name, buf, this->Handle());
}

void Config::WriteString(char* Cata, char* Name, char* SetVal)
{
	WritePrivateProfileStringA(Cata, Name, SetVal, this->Handle());
}

void Config::SaveConfig()
{
	WriteInt("Rage (aimbot)", "Enabled", g_Settings->rage_Enabled);
	WriteInt("Rage (aimbot)", "Onkey", g_Settings->rage_Onkey);
	WriteInt("Rage (aimbot)", "Key", g_Settings->rage_Key);
	WriteInt("Rage (aimbot)", "MultipointEnabled", g_Settings->rage_MultipointEnable);
	WriteFloat("Rage (aimbot)", "Headscale", g_Settings->rage_MultipHead);
	WriteFloat("Rage (aimbot)", "Bodyscale", g_Settings->rage_MultipBody);
	WriteInt("Rage (aimbot)", "Hitscan", g_Settings->rage_Hitscan);
	WriteFloat("Rage (aimbot)", "Hitchance", g_Settings->rage_Hitchance);
	WriteFloat("Rage (aimbot)", "Mindamage", g_Settings->rage_Mindamage);
	WriteInt("Rage (aimbot)", "Autofire", g_Settings->rage_AutoShot);
	WriteInt("Rage (aimbot)", "Autoscope", g_Settings->rage_AutoScope);
	WriteInt("Rage (aimbot)", "Autor8", g_Settings->rage_AutoRevolver);
	WriteInt("Rage (aimbot)", "Removerecoil", g_Settings->rage_Norecoil);
	WriteInt("Rage (aimbot)", "Resolver", g_Settings->rage_Resolver);
	WriteInt("Rage (aimbot)", "ClearRecs", g_Settings->rage_ResolverClearRecs);
	WriteInt("Rage (antiaim)", "Enabled", g_Settings->aa_Enabled);
	WriteInt("Rage (antiaim)", "Disableknife", g_Settings->aa_DisableKnife);
	WriteInt("Rage (antiaim)", "Pitch", g_Settings->aa_Pitch);
	WriteInt("Rage (antiaim)", "Ryawstand", g_Settings->aa_RYawStand);
	WriteInt("Rage (antiaim)", "Ryawstandang", g_Settings->aa_RYawAngStand);
	WriteInt("Rage (antiaim)", "Ryawmove", g_Settings->aa_RYawMove);
	WriteInt("Rage (antiaim)", "Ryawmoveang", g_Settings->aa_RYawAngMove);
	WriteInt("Rage (antiaim)", "Fyawbase", g_Settings->aa_FYawBase);
	WriteInt("Rage (antiaim)", "Yawbase", g_Settings->aa_YawBase);
	WriteInt("Rage (antiaim)", "Fyaw", g_Settings->aa_FYaw);
	WriteInt("Rage (antiaim)", "Fyawang", g_Settings->aa_FYawAng);
	WriteInt("Rage (antiaim)", "Breaker", g_Settings->aa_Breakertype);
	WriteInt("Rage (antiaim)", "BreakLby", g_Settings->aa_BreakLby);
	WriteInt("Rage (antiaim)", "Reverse_key", g_Settings->aa_Reversekey);
	WriteInt("Rage (fakelags)", "Enabled", g_Settings->rage_LagsEnabled);
	WriteInt("Rage (fakelags)", "Max", g_Settings->rage_LagsMax);
	WriteInt("Rage (fakelags)", "Min", g_Settings->rage_LagsMin);
	WriteInt("Rage (fakelags)", "Air", g_Settings->rage_LagsAir);

	WriteInt(("Visuals (player)"), ("Enabled"), g_Settings->visuals_Enabled);
	WriteFloat(("Visuals (player)"), ("Opacity"), g_Settings->visuals_Opacity);
	WriteInt(("Visuals (player)"), ("Enemyonly"), g_Settings->visuals_EnemyOnly);
	WriteInt(("Visuals (player)"), ("Box"), g_Settings->visuals_BoundingBox);
	WriteInt(("Visuals (player)"), ("Bones"), g_Settings->visuals_Bones);
	WriteInt(("Visuals (player)"), ("Health"), g_Settings->visuals_Health);
	WriteInt(("Visuals (player)"), ("Name"), g_Settings->visuals_Name);
	WriteInt(("Visuals (player)"), ("Weapon"), g_Settings->visuals_Weapon);
	WriteInt(("Visuals (player)"), ("Ammo"), g_Settings->visuals_Ammo);
	WriteInt(("Visuals (player)"), ("Resolverinfo"), g_Settings->visuals_ResolverInfo);
	WriteInt(("Visuals (player)"), ("Botinfo"), g_Settings->visuals_BotInfo);
	WriteInt(("Visuals (player)"), ("Hitmarker"), g_Settings->visuals_Hitmarker);
	WriteInt(("Visuals (player)"), ("Glow"), g_Settings->visuals_Glow);
	WriteInt(("Visuals (player)"), ("Offscreen"), g_Settings->visuals_Offscreen);
	WriteInt(("Visuals (player chams)"), ("Enabled"), g_Settings->visuals_ChamsEnabled);
	WriteInt(("Visuals (player chams)"), ("Style"), g_Settings->visuals_ChamsStyle);
	WriteInt(("Visuals (player chams)"), ("Visible"), g_Settings->visuals_Chams);
	WriteInt(("Visuals (player chams)"), ("Invisible"), g_Settings->visuals_ChamsWall);
	WriteInt(("Visuals (weapon)"), ("Dmgindicator"), g_Settings->visuals_DamageIndicator);
	WriteInt(("Visuals (weapon)"), ("Grenadepred"), g_Settings->visuals_GrenadePrediction);
	WriteInt(("Visuals (weapon)"), ("Spread"), g_Settings->visuals_SpreadCircle);
	WriteInt(("Visuals (weapon)"), ("Tracer"), g_Settings->visuals_BulletTracers);
	WriteInt(("Visuals (weapon)"), ("Box"), g_Settings->visuals_entBox);
	WriteInt(("Visuals (weapon)"), ("Name"), g_Settings->visuals_entName);
	WriteInt(("Visuals (weapon)"), ("Glow"), g_Settings->visuals_entGlow);
	WriteInt(("Visuals (weapon)"), ("Weapon"), g_Settings->visuals_DroppedWeapon);
	WriteInt(("Visuals (weapon)"), ("Bomb"), g_Settings->visuals_PlantedBomb);
	WriteInt(("Visuals (weapon)"), ("Nades"), g_Settings->visuals_Nades);
	WriteInt(("Visuals (effects)"), ("Nightmode"), g_Settings->visuals_Nightmode);
	WriteInt(("Visuals (effects)"), ("Skybox"), g_Settings->visuals_Skybox);
	WriteInt(("Visuals (effects)"), ("Removeflash"), g_Settings->visuals_NoFlash);
	WriteInt(("Visuals (effects)"), ("Removesmoke"), g_Settings->visuals_NoSmoke);
	WriteInt(("Visuals (effects)"), ("Removescope"), g_Settings->visuals_NoScope);
	WriteInt(("Visuals (effects)"), ("Removeblur"), g_Settings->visuals_NoBlur);
	WriteInt(("Visuals (effects)"), ("Removevisrecoil"), g_Settings->visuals_VisNoRecoil);
	WriteInt(("Visuals (effects)"), ("Postproc"), g_Settings->visuals_PostProc);
	WriteInt(("Visuals (effects)"), ("Ghost"), g_Settings->visuals_FakeAngleGhost);
	WriteInt(("Visuals (effects)"), ("Tpangles"), g_Settings->misc_TPangles);
	WriteInt(("Visuals (effects)"), ("Tpkey"), g_Settings->misc_TPKey);
	WriteInt(("Visuals (effects)"), ("Showreal"), g_Settings->visuals_ShowReal);
	WriteInt(("Visuals (effects)"), ("Showfake"), g_Settings->visuals_ShowFake);
	WriteInt(("Visuals (effects)"), ("Showlby"), g_Settings->visuals_ShowLby);

	WriteInt(("Misc"), ("Antiuntrust"), g_Settings->misc_AntiUntrust);
	WriteInt(("Misc"), ("Fov"), g_Settings->misc_FOV);
	WriteInt(("Misc"), ("Viewmodel"), g_Settings->misc_Viewmodel);
	WriteInt(("Misc"), ("Menukey"), g_Settings->misc_MenuKey);
	WriteInt(("Misc"), ("Bhop"), g_Settings->misc_Bhop);
	WriteInt(("Misc"), ("Strafe"), g_Settings->misc_AutoStrafe);
	WriteInt(("Misc"), ("Prespeed"), g_Settings->misc_Prespeed);
	WriteInt(("Misc"), ("Prespeedkey"), g_Settings->misc_PrespeedKey);
	WriteInt(("Misc"), ("Retrack"), g_Settings->misc_Retrack);
	WriteInt(("Misc"), ("Fakewalk"), g_Settings->misc_FakewalkEnabled);
	WriteInt(("Misc"), ("Fakewalkkey"), g_Settings->misc_FakewalkKey);
	WriteInt(("Misc"), ("Clantag"), g_Settings->misc_Clantag);
	WriteInt(("Misc"), ("KbotEnabled"), g_Settings->misc_KbEnabled);
	WriteInt(("Misc"), ("KbotBack"), g_Settings->misc_KbBackOnly);
	WriteInt(("Misc"), ("KbotCrouch"), g_Settings->misc_KbCrouch);
	WriteInt(("Misc"), ("Logdmg"), g_Settings->misc_Logdmg);

	//Menu
	WriteFloat(("Colors"), ("MenuR"), g_Settings->color_Menu[0]);
	WriteFloat(("Colors"), ("MenuG"), g_Settings->color_Menu[1]);
	WriteFloat(("Colors"), ("MenuB"), g_Settings->color_Menu[2]);
	// Box
	WriteFloat(("Colors"), ("BoxR"), g_Settings->color_BoundingBox[0]);
	WriteFloat(("Colors"), ("BoxG"), g_Settings->color_BoundingBox[1]);
	WriteFloat(("Colors"), ("BoxB"), g_Settings->color_BoundingBox[2]);
	// Chams vis
	WriteFloat(("Colors"), ("ChamsVisR"), g_Settings->color_Chams[0]);
	WriteFloat(("Colors"), ("ChamsVisG"), g_Settings->color_Chams[1]);
	WriteFloat(("Colors"), ("ChamsVisB"), g_Settings->color_Chams[2]);
	// Chams invis
	WriteFloat(("Colors"), ("ChamsInvisR"), g_Settings->color_ChamsWall[0]);
	WriteFloat(("Colors"), ("ChamsInvisG"), g_Settings->color_ChamsWall[1]);
	WriteFloat(("Colors"), ("ChamsInvisB"), g_Settings->color_ChamsWall[2]);
	// Fake angles
	WriteFloat(("Colors"), ("GhostR"), g_Settings->color_FakeAngleGhost[0]);
	WriteFloat(("Colors"), ("GhostG"), g_Settings->color_FakeAngleGhost[1]);
	WriteFloat(("Colors"), ("GhostB"), g_Settings->color_FakeAngleGhost[2]);
	// Skeleton
	WriteFloat(("Colors"), ("SkeletonR"), g_Settings->color_Skeletons[0]);
	WriteFloat(("Colors"), ("SkeletonG"), g_Settings->color_Skeletons[1]);
	WriteFloat(("Colors"), ("SkeletonB"), g_Settings->color_Skeletons[2]);
	// Bullet tracer
	WriteFloat(("Colors"), ("TracerR"), g_Settings->color_Bullettracer[0]);
	WriteFloat(("Colors"), ("TracerG"), g_Settings->color_Bullettracer[1]);
	WriteFloat(("Colors"), ("TracerB"), g_Settings->color_Bullettracer[2]);
	// Glow
	WriteFloat(("Colors"), ("GlowR"), g_Settings->color_Glow[0]);
	WriteFloat(("Colors"), ("GlowG"), g_Settings->color_Glow[1]);
	WriteFloat(("Colors"), ("GlowB"), g_Settings->color_Glow[2]);
	// Spread
	WriteFloat(("Colors"), ("SpreadR"), g_Settings->color_SpreadCircle[0]);
	WriteFloat(("Colors"), ("SpreadG"), g_Settings->color_SpreadCircle[1]);
	WriteFloat(("Colors"), ("SpreadB"), g_Settings->color_SpreadCircle[2]);
	// Damage
	WriteFloat(("Colors"), ("DamageR"), g_Settings->color_DamageIndicator[0]);
	WriteFloat(("Colors"), ("DamageG"), g_Settings->color_DamageIndicator[1]);
	WriteFloat(("Colors"), ("DamageB"), g_Settings->color_DamageIndicator[2]);
	// Ammo
	WriteFloat(("Colors"), ("AmmoR"), g_Settings->color_Ammobar[0]);
	WriteFloat(("Colors"), ("AmmoG"), g_Settings->color_Ammobar[1]);
	WriteFloat(("Colors"), ("AmmoB"), g_Settings->color_Ammobar[2]);
	// Entity box
	WriteFloat(("Colors"), ("EntBoxR"), g_Settings->color_entBox[0]);
	WriteFloat(("Colors"), ("EntBoxG"), g_Settings->color_entBox[1]);
	WriteFloat(("Colors"), ("EntBoxB"), g_Settings->color_entBox[2]);
	// Entity name
	WriteFloat(("Colors"), ("EntNameR"), g_Settings->color_entName[0]);
	WriteFloat(("Colors"), ("EntNameG"), g_Settings->color_entName[1]);
	WriteFloat(("Colors"), ("EntNameB"), g_Settings->color_entName[2]);
	// Entity glow
	WriteFloat(("Colors"), ("EntGlowR"), g_Settings->color_entGlow[0]);
	WriteFloat(("Colors"), ("EntGlowG"), g_Settings->color_entGlow[1]);
	WriteFloat(("Colors"), ("EntGlowB"), g_Settings->color_entGlow[2]);
	// Resolver info
	WriteFloat(("Colors"), ("ResolverinfoR"), g_Settings->color_ResolverInfo[0]);
	WriteFloat(("Colors"), ("ResolverinfoG"), g_Settings->color_ResolverInfo[1]);
	WriteFloat(("Colors"), ("ResolverinfoB"), g_Settings->color_ResolverInfo[2]);
	// Bot info
	WriteFloat(("Colors"), ("BotinfoR"), g_Settings->color_BotInfo[0]);
	WriteFloat(("Colors"), ("BotinfoG"), g_Settings->color_BotInfo[1]);
	WriteFloat(("Colors"), ("BotinfoB"), g_Settings->color_BotInfo[2]);
	// Off screen
	WriteFloat(("Colors"), ("OffSR"), g_Settings->color_OffScreen[0]);
	WriteFloat(("Colors"), ("OffSG"), g_Settings->color_OffScreen[1]);
	WriteFloat(("Colors"), ("OffSB"), g_Settings->color_OffScreen[2]);
}

#define CfgReadInt(type, group, name) { type = ReadInt((group), (name), 0); }
#define CfgReadFloat(type, group, name) { type = ReadFloat((group), (name), 0.f); }

void Config::LoadConfig()
{
	CfgReadInt(g_Settings->rage_Enabled, "Rage (aimbot)", "Enabled");
	CfgReadInt(g_Settings->rage_Onkey, "Rage (aimbot)", "Onkey");
	CfgReadInt(g_Settings->rage_Key, "Rage (aimbot)", "Key");
	CfgReadInt(g_Settings->rage_MultipointEnable, "Rage (aimbot)", "MultipointEnabled");
	CfgReadFloat(g_Settings->rage_MultipHead, "Rage (aimbot)", "Headscale");
	CfgReadFloat(g_Settings->rage_MultipBody, "Rage (aimbot)", "Bodyscale");
	CfgReadInt(g_Settings->rage_Hitscan, "Rage (aimbot)", "Hitscan");
	CfgReadInt(g_Settings->rage_AutoRevolver, "Rage (aimbot)", "Autor8");
	CfgReadFloat(g_Settings->rage_Hitchance, "Rage (aimbot)", "Hitchance");
	CfgReadFloat(g_Settings->rage_Mindamage, "Rage (aimbot)", "Mindamage");
	CfgReadInt(g_Settings->rage_AutoShot, "Rage (aimbot)", "Autofire");
	CfgReadInt(g_Settings->rage_AutoScope, "Rage (aimbot)", "Autoscope");
	CfgReadInt(g_Settings->rage_Norecoil, "Rage (aimbot)", "Removerecoil");
	CfgReadInt(g_Settings->rage_Resolver, "Rage (aimbot)", "Resolver");
	CfgReadInt(g_Settings->rage_ResolverClearRecs, "Rage (aimbot)", "ClearRecs");
	CfgReadInt(g_Settings->aa_Enabled, "Rage (antiaim)", "Enabled");
	CfgReadInt(g_Settings->aa_DisableKnife, "Rage (antiaim)", "Disableknife");
	CfgReadInt(g_Settings->aa_Pitch, "Rage (antiaim)", "Pitch");
	CfgReadInt(g_Settings->aa_RYawStand, "Rage (antiaim)", "Ryawstand");
	CfgReadFloat(g_Settings->aa_RYawAngStand, "Rage (antiaim)", "Ryawstandang");
	CfgReadInt(g_Settings->aa_RYawMove, "Rage (antiaim)", "Ryawmove");
	CfgReadFloat(g_Settings->aa_RYawAngMove, "Rage (antiaim)", "Ryawmoveang");
	CfgReadInt(g_Settings->aa_FYawBase, "Rage (antiaim)", "Fyawbase");
	CfgReadInt(g_Settings->aa_YawBase, "Rage (antiaim)", "Yawbase");
	CfgReadInt(g_Settings->aa_FYaw, "Rage (antiaim)", "Fyaw");
	CfgReadFloat(g_Settings->aa_FYawAng, "Rage (antiaim)", "Fyawang");
	CfgReadInt(g_Settings->aa_BreakLby, "Rage (antiaim)", "BreakLby");
	CfgReadInt(g_Settings->aa_Breakertype, "Rage (antiaim)", "Breaker");
	CfgReadInt(g_Settings->aa_Reversekey, "Rage (antiaim)", "Reverse_key");
	CfgReadInt(g_Settings->rage_LagsEnabled, "Rage (fakelags)", "Enabled");
	CfgReadInt(g_Settings->rage_LagsMax, "Rage (fakelags)", "Max");
	CfgReadInt(g_Settings->rage_LagsMin, "Rage (fakelags)", "Min");
	CfgReadInt(g_Settings->rage_LagsAir, "Rage (fakelags)", "Air");

	CfgReadInt(g_Settings->visuals_Enabled, "Visuals (player)", "Enabled");
	CfgReadFloat(g_Settings->visuals_Opacity, "Visuals (player)", "Opacity");
	CfgReadInt(g_Settings->visuals_EnemyOnly, "Visuals (player)", "Enemyonly");
	CfgReadInt(g_Settings->visuals_BoundingBox, "Visuals (player)", "Box");
	CfgReadInt(g_Settings->visuals_Bones, "Visuals (player)", "Bones");
	CfgReadInt(g_Settings->visuals_Health, "Visuals (player)", "Health");
	CfgReadInt(g_Settings->visuals_Name, "Visuals (player)", "Name");
	CfgReadInt(g_Settings->visuals_Weapon, "Visuals (player)", "Weapon");
	CfgReadInt(g_Settings->visuals_Ammo, "Visuals (player)", "Ammo");
	CfgReadInt(g_Settings->visuals_ResolverInfo, "Visuals (player)", "Resolverinfo");
	CfgReadInt(g_Settings->visuals_BotInfo, "Visuals (player)", "Botinfo");
	CfgReadInt(g_Settings->visuals_Hitmarker, "Visuals (player)", "Hitmarker");
	CfgReadInt(g_Settings->visuals_Glow, "Visuals (player)", "Glow");
	CfgReadInt(g_Settings->visuals_Offscreen, "Visuals (player)", "Offscreen");
	CfgReadInt(g_Settings->visuals_ChamsEnabled, "Visuals (player chams)", "Enabled");
	CfgReadInt(g_Settings->visuals_ChamsStyle, "Visuals (player chams)", "Style");
	CfgReadInt(g_Settings->visuals_Chams, "Visuals (player chams)", "Visible");
	CfgReadInt(g_Settings->visuals_ChamsWall, "Visuals (player chams)", "Invisible");
	CfgReadInt(g_Settings->visuals_DamageIndicator, "Visuals (weapon)", "Dmgindicator");
	CfgReadInt(g_Settings->visuals_GrenadePrediction, "Visuals (weapon)", "Grenadepred");
	CfgReadInt(g_Settings->visuals_SpreadCircle, "Visuals (weapon)", "Spread");
	CfgReadInt(g_Settings->visuals_BulletTracers, "Visuals (weapon)", "Tracer");
	CfgReadInt(g_Settings->visuals_entBox, "Visuals (weapon)", "Box");
	CfgReadInt(g_Settings->visuals_entName, "Visuals (weapon)", "Name");
	CfgReadInt(g_Settings->visuals_entGlow, "Visuals (weapon)", "Glow");
	CfgReadInt(g_Settings->visuals_DroppedWeapon, "Visuals (weapon)", "Weapon");
	CfgReadInt(g_Settings->visuals_PlantedBomb, "Visuals (weapon)", "Bomb");
	CfgReadInt(g_Settings->visuals_Nades, "Visuals (weapon)", "Nades");
	CfgReadInt(g_Settings->visuals_Nightmode, "Visuals (effects)", "Nightmode");
	CfgReadInt(g_Settings->visuals_Skybox, "Visuals (effects)", "Skybox");
	CfgReadInt(g_Settings->visuals_NoFlash, "Visuals (effects)", "Removeflash");
	CfgReadInt(g_Settings->visuals_NoSmoke, "Visuals (effects)", "Removesmoke");
	CfgReadInt(g_Settings->visuals_NoScope, "Visuals (effects)", "Removescope");
	CfgReadInt(g_Settings->visuals_NoBlur, "Visuals (effects)", "Removeblur");
	CfgReadInt(g_Settings->visuals_VisNoRecoil, "Visuals (effects)", "Removevisrecoil");
	CfgReadInt(g_Settings->visuals_PostProc, "Visuals (effects)", "Postproc");
	CfgReadInt(g_Settings->visuals_FakeAngleGhost, "Visuals (effects)", "Ghost");
	CfgReadInt(g_Settings->misc_TPangles, "Visuals (effects)", "Tpangles");
	CfgReadInt(g_Settings->misc_TPKey, "Visuals (effects)", "Tpkey");
	CfgReadInt(g_Settings->visuals_ShowReal, "Visuals (effects)", "Showreal");
	CfgReadInt(g_Settings->visuals_ShowFake, "Visuals (effects)", "Showfake");
	CfgReadInt(g_Settings->visuals_ShowLby, "Visuals (effects)", "Showlby");

	CfgReadInt(g_Settings->misc_AntiUntrust, "Misc", "Antiuntrust");
	CfgReadInt(g_Settings->misc_FOV, "Misc", "Fov");
	CfgReadInt(g_Settings->misc_Viewmodel, "Misc", "Viewmodel");
	CfgReadInt(g_Settings->misc_MenuKey, "Misc", "Menukey");
	CfgReadInt(g_Settings->misc_Bhop, "Misc", "Bhop");
	CfgReadInt(g_Settings->misc_AutoStrafe, "Misc", "Strafe");
	CfgReadInt(g_Settings->misc_Prespeed, "Misc", "Prespeed");
	CfgReadInt(g_Settings->misc_PrespeedKey, "Misc", "Prespeedkey");
	CfgReadInt(g_Settings->misc_Retrack, "Misc", "Retrack");
	CfgReadInt(g_Settings->misc_FakewalkEnabled, "Misc", "Fakewalk");
	CfgReadInt(g_Settings->misc_FakewalkKey, "Misc", "Fakewalkkey");
	CfgReadInt(g_Settings->misc_Clantag, "Misc", "Clantag");
	CfgReadInt(g_Settings->misc_KbEnabled, "Misc", "KbotEnabled");
	CfgReadInt(g_Settings->misc_KbBackOnly, "Misc", "KbotBack");
	CfgReadInt(g_Settings->misc_KbCrouch, "Misc", "KbotCrouch");
	CfgReadInt(g_Settings->misc_Logdmg, "Misc", "Logdmg");

	CfgReadFloat(g_Settings->color_Menu[0], "Colors", "MenuR"); CfgReadFloat(g_Settings->color_Menu[1], "Colors", "MenuG"); CfgReadFloat(g_Settings->color_Menu[2], "Colors", "MenuB");
	CfgReadFloat(g_Settings->color_BoundingBox[0], "Colors", "BoxR"); CfgReadFloat(g_Settings->color_BoundingBox[1], "Colors", "BoxG"); CfgReadFloat(g_Settings->color_BoundingBox[2], "Colors", "BoxB");
	CfgReadFloat(g_Settings->color_Chams[0], "Colors", "ChamsVisR"); CfgReadFloat(g_Settings->color_Chams[1], "Colors", "ChamsVisG"); CfgReadFloat(g_Settings->color_Chams[2], "Colors", "ChamsVisB");
	CfgReadFloat(g_Settings->color_ChamsWall[0], "Colors", "ChamsInvisR"); CfgReadFloat(g_Settings->color_ChamsWall[1], "Colors", "ChamsInvisG"); CfgReadFloat(g_Settings->color_ChamsWall[2], "Colors", "ChamsInvisB");
	CfgReadFloat(g_Settings->color_FakeAngleGhost[0], "Colors", "GhostR"); CfgReadFloat(g_Settings->color_FakeAngleGhost[1], "Colors", "GhostG"); CfgReadFloat(g_Settings->color_FakeAngleGhost[2], "Colors", "GhostB");
	CfgReadFloat(g_Settings->color_Skeletons[0], "Colors", "SkeletonR"); CfgReadFloat(g_Settings->color_Skeletons[1], "Colors", "SkeletonG"); CfgReadFloat(g_Settings->color_Skeletons[2], "Colors", "SkeletonB");
	CfgReadFloat(g_Settings->color_Bullettracer[0], "Colors", "TracerR"); CfgReadFloat(g_Settings->color_Bullettracer[1], "Colors", "TracerG"); CfgReadFloat(g_Settings->color_Bullettracer[2], "Colors", "TracerB");
	CfgReadFloat(g_Settings->color_Glow[0], "Colors", "GlowR"); CfgReadFloat(g_Settings->color_Glow[1], "Colors", "GlowG"); CfgReadFloat(g_Settings->color_Glow[2], "Colors", "GlowB");
	CfgReadFloat(g_Settings->color_SpreadCircle[0], "Colors", "SpreadR"); CfgReadFloat(g_Settings->color_SpreadCircle[1], "Colors", "SpreadG"); CfgReadFloat(g_Settings->color_SpreadCircle[2], "Colors", "SpreadB");
	CfgReadFloat(g_Settings->color_DamageIndicator[0], "Colors", "DamageR"); CfgReadFloat(g_Settings->color_DamageIndicator[1], "Colors", "DamageG"); CfgReadFloat(g_Settings->color_DamageIndicator[2], "Colors", "DamageB");
	CfgReadFloat(g_Settings->color_Ammobar[0], "Colors", "AmmoR"); CfgReadFloat(g_Settings->color_Ammobar[1], "Colors", "AmmoG"); CfgReadFloat(g_Settings->color_Ammobar[2], "Colors", "AmmoB");
	CfgReadFloat(g_Settings->color_entBox[0], "Colors", "EntBoxR"); CfgReadFloat(g_Settings->color_entBox[1], "Colors", "EntBoxG"); CfgReadFloat(g_Settings->color_entBox[2], "Colors", "EntBoxB");
	CfgReadFloat(g_Settings->color_entName[0], "Colors", "EntNameR"); CfgReadFloat(g_Settings->color_entName[1], "Colors", "EntNameG"); CfgReadFloat(g_Settings->color_entName[2], "Colors", "EntNameB");
	CfgReadFloat(g_Settings->color_entGlow[0], "Colors", "EntGlowR"); CfgReadFloat(g_Settings->color_entGlow[1], "Colors", "EntGlowG"); CfgReadFloat(g_Settings->color_entGlow[2], "Colors", "EntGlowB");
	CfgReadFloat(g_Settings->color_ResolverInfo[0], "Colors", "ResolverinfoR"); CfgReadFloat(g_Settings->color_ResolverInfo[1], "Colors", "ResolverinfoG"); CfgReadFloat(g_Settings->color_ResolverInfo[2], "Colors", "ResolverinfoB");
	CfgReadFloat(g_Settings->color_BotInfo[0], "Colors", "BotinfoR"); CfgReadFloat(g_Settings->color_BotInfo[1], "Colors", "BotinfoG"); CfgReadFloat(g_Settings->color_BotInfo[2], "Colors", "BotinfoB");
	CfgReadFloat(g_Settings->color_OffScreen[0], "Colors", "OffSR"); CfgReadFloat(g_Settings->color_OffScreen[1], "Colors", "OffSG"); CfgReadFloat(g_Settings->color_OffScreen[2], "Colors", "OffSB");
}

void Config::Reset()
{
	g_Settings->rage_Enabled = false;
	g_Settings->rage_Onkey = false;
	g_Settings->rage_Key = 0;
	g_Settings->rage_Hitscan = 0;
	g_Settings->rage_AutoRevolver = false;
	g_Settings->rage_AutoShot = false;
	g_Settings->rage_AutoScope = false;
	g_Settings->rage_Norecoil = false;
	g_Settings->rage_Mindamage = 0;
	g_Settings->rage_Hitchance = 0;
	g_Settings->rage_MultipointEnable = false;
	g_Settings->rage_MultipHead = 0;
	g_Settings->rage_MultipBody = 0;
	g_Settings->rage_Resolver = false;
	g_Settings->rage_ResolverClearRecs = true;
	g_Settings->aa_Enabled = false;
	g_Settings->aa_DisableKnife = false;
	g_Settings->aa_Pitch = 0;
	g_Settings->aa_RYawStand = 0;
	g_Settings->aa_RYawAngStand = 0;
	g_Settings->aa_RYawMove = 0;
	g_Settings->aa_RYawAngMove = 0;
	g_Settings->aa_YawBase = 0;
	g_Settings->aa_FYawBase = 0;
	g_Settings->aa_FYaw = 0;
	g_Settings->aa_FYawAng = 0;
	g_Settings->aa_Breakertype = 0;
	g_Settings->aa_Reversekey = 0;
	g_Settings->rage_LagsEnabled = false;
	g_Settings->rage_LagsMax = 0;
	g_Settings->rage_LagsMin = 0;
	g_Settings->rage_LagsAir = 0;

	g_Settings->visuals_Enabled = false;
	g_Settings->visuals_Opacity = 255;
	g_Settings->visuals_EnemyOnly = false;
	g_Settings->visuals_BoundingBox = false;
	g_Settings->visuals_Bones = false;
	g_Settings->visuals_Health = false;
	g_Settings->visuals_Name = false;
	g_Settings->visuals_Weapon = false;
	g_Settings->visuals_Ammo = false;
	g_Settings->visuals_ResolverInfo = false;
	g_Settings->visuals_BotInfo = false;
	g_Settings->visuals_Hitmarker = false;
	g_Settings->visuals_DamageIndicator = false;
	g_Settings->visuals_Glow = false;
	g_Settings->visuals_Offscreen = false;
	g_Settings->visuals_GrenadePrediction = false;
	g_Settings->visuals_SpreadCircle = false;
	g_Settings->visuals_BulletTracers = false;
	g_Settings->visuals_entBox = false;
	g_Settings->visuals_entName = false;
	g_Settings->visuals_entGlow = false;
	g_Settings->visuals_DroppedWeapon = false;
	g_Settings->visuals_PlantedBomb = false;
	g_Settings->visuals_Nades = false;
	g_Settings->visuals_Nightmode = false;
	g_Settings->visuals_Skybox = false;
	g_Settings->visuals_NoFlash = false;
	g_Settings->visuals_NoSmoke = false;
	g_Settings->visuals_NoScope = false;
	g_Settings->visuals_NoBlur = false;
	g_Settings->visuals_PostProc = false;
	g_Settings->visuals_VisNoRecoil = false;
	g_Settings->visuals_FakeAngleGhost = 0;
	g_Settings->visuals_ShowReal = false;
	g_Settings->visuals_ShowFake = false;
	g_Settings->visuals_ShowLby = false;
	g_Settings->visuals_ChamsEnabled = false;
	g_Settings->visuals_ChamsStyle = 0;
	g_Settings->visuals_Chams = false;
	g_Settings->visuals_ChamsWall = false;

	g_Settings->misc_AntiUntrust = true;
	g_Settings->misc_FOV = 0.f;
	g_Settings->misc_Viewmodel = 0.f;
	g_Settings->misc_TPangles = 0;
	g_Settings->misc_TPKey = 0;
	g_Settings->misc_MenuKey = 0x2d;
	g_Settings->misc_ConfigSelection = 0;
	g_Settings->misc_Bhop = false;
	g_Settings->misc_AutoStrafe = false;
	g_Settings->misc_Prespeed = false;
	g_Settings->misc_Retrack = 1.f;
	g_Settings->misc_PrespeedKey = 0;
	g_Settings->misc_FakewalkEnabled = false;
	g_Settings->misc_FakewalkKey = 0;
	g_Settings->misc_Clantag = false;
	g_Settings->misc_KbEnabled = false;
	g_Settings->misc_KbBackOnly = false;
	g_Settings->misc_KbCrouch = false;
	g_Settings->misc_Logdmg = false;

	g_Settings->color_Menu[0] = 0.074f; g_Settings->color_Menu[1] = 0.254f; g_Settings->color_Menu[2] = 0.745f;
	g_Settings->color_BoundingBox[0] = 0.074f; g_Settings->color_BoundingBox[1] = 0.254f; g_Settings->color_BoundingBox[2] = 0.745f;
	g_Settings->color_Chams[0] = 0.074f; g_Settings->color_Chams[1] = 0.254f; g_Settings->color_Chams[2] = 0.745f;
	g_Settings->color_ChamsWall[0] = 0.074f; g_Settings->color_ChamsWall[1] = 0.254f; g_Settings->color_ChamsWall[2] = 0.745f;
	g_Settings->color_FakeAngleGhost[0] = 1.f; g_Settings->color_FakeAngleGhost[1] = 1.f; g_Settings->color_FakeAngleGhost[2] = 1.f;
	g_Settings->color_Skeletons[0] = 1.f; g_Settings->color_Skeletons[1] = 1.f; g_Settings->color_Skeletons[2] = 1.f;
	g_Settings->color_Bullettracer[0] = 0.611f; g_Settings->color_Bullettracer[1] = 0.050f; g_Settings->color_Bullettracer[2] = 0.627f;
	g_Settings->color_Glow[0] = 0.074f; g_Settings->color_Glow[1] = 0.254f; g_Settings->color_Glow[2] = 0.745f;
	g_Settings->color_SpreadCircle[0] = 0.074f; g_Settings->color_SpreadCircle[1] = 0.254f; g_Settings->color_SpreadCircle[2] = 0.745f;
	g_Settings->color_DamageIndicator[0] = 1.f; g_Settings->color_DamageIndicator[1] = 1.f; g_Settings->color_DamageIndicator[2] = 1.f;
	g_Settings->color_Ammobar[0] = 0.611f; g_Settings->color_Ammobar[1] = 0.050f; g_Settings->color_Ammobar[2] = 0.627f;
	g_Settings->color_entBox[0] = 0.074f; g_Settings->color_entBox[1] = 0.254f; g_Settings->color_entBox[2] = 0.745f;
	g_Settings->color_entName[0] = 0.074f; g_Settings->color_entName[1] = 0.254f; g_Settings->color_entName[2] = 0.745f;
	g_Settings->color_entGlow[0] = 0.074f; g_Settings->color_entGlow[1] = 0.254f; g_Settings->color_entGlow[2] = 0.745f;
	g_Settings->color_ResolverInfo[0] = 1.f; g_Settings->color_ResolverInfo[1] = 1.f; g_Settings->color_ResolverInfo[2] = 1.f;
	g_Settings->color_BotInfo[0] = 1.f; g_Settings->color_BotInfo[1] = 1.f; g_Settings->color_BotInfo[2] = 1.f;
	g_Settings->color_OffScreen[0] = 0.074f; g_Settings->color_OffScreen[1] = 0.254f; g_Settings->color_OffScreen[2] = 0.745f;
}