#pragma once


class CSettings
{
public:
	/*test*/
	bool test_Animtest;
	int test_Animcount;


	/*rage*/
	bool rage_Enabled;
	bool rage_Onkey;
	int rage_Key;
	bool rage_AutoShot;
	bool rage_AutoScope;
	bool rage_AutoRevolver;
	bool rage_Norecoil;
	int rage_Hitscan;
	bool rage_Hitboxes[18] =
	{
		true, true, true, true,
		true, true, true, true,
		true, true, true, true,
		true, true, true, true,
		true, true
	};
	int rage_Selection;
	float rage_Mindamage;
	float rage_Hitchance;
	float rage_MultipHead, rage_MultipBody, rage_MultipDistance = 90.f;
	bool rage_MultipointEnable;
	bool rage_Resolver, rage_ResolverClearRecs = true;
	bool rage_LagsEnabled;
	int rage_LagsMax, rage_LagsMin, rage_LagsAir;
	bool aa_Enabled;
	bool aa_DisableKnife;
	int aa_Pitch, aa_RYawStand, aa_RYawMove, aa_FYaw;
	float aa_FYawAng, aa_RYawAngStand, aa_RYawAngMove;
	int aa_FYawBase, aa_YawBase;
	bool aa_BreakLby;
	int aa_Breakertype;
	int aa_Reversekey;

	/*visuals*/
	bool visuals_Enabled;
	int visuals_Opacity = 255;
	bool visuals_EnemyOnly;
	bool visuals_BoundingBox;
	bool visuals_ResolverInfo;
	bool visuals_BotInfo;
	bool visuals_Bones;
	bool visuals_Health;
	bool visuals_Name;
	int visuals_Weapon;
	bool visuals_Ammo;
	bool visuals_Hitmarker;
	bool visuals_DamageIndicator;
	bool visuals_Glow;
	bool visuals_GrenadePrediction;
	bool visuals_SpreadCircle;
	bool visuals_Offscreen;
	//Cbase
	bool visuals_BulletTracers;
	bool visuals_entBox, visuals_entName, visuals_entGlow;
	bool visuals_DroppedWeapon, visuals_PlantedBomb, visuals_Nades;
	//Effects
	bool visuals_Nightmode;
	int visuals_Skybox;
	bool visuals_NoFlash, visuals_NoSmoke, visuals_NoScope, visuals_NoBlur, visuals_PostProc, visuals_VisNoRecoil;
	int visuals_FakeAngleGhost;
	bool visuals_ShowReal, visuals_ShowFake, visuals_ShowLby;
	//Chams
	bool visuals_ChamsEnabled;
	int visuals_ChamsStyle;
	bool visuals_Chams;
	bool visuals_ChamsWall;

	/*misc*/
	bool misc_AntiUntrust = true;
	int misc_FOV = 0.f;
	int misc_Viewmodel = 0.f;
	int misc_TPangles = 0;
	int misc_TPKey = 0;
	int misc_MenuKey = 0x2d;
	int misc_ConfigSelection = 0;
	bool misc_Bhop;
	bool misc_AutoStrafe;
	bool misc_Prespeed;
	int misc_Retrack = 1.f;
	int misc_PrespeedKey = 0;
	bool misc_FakewalkEnabled;
	int misc_FakewalkKey = 0;
	bool misc_Clantag;
	bool misc_KbEnabled, misc_KbBackOnly, misc_KbCrouch;
	bool misc_Logdmg;

	/*colors*/
	float color_Menu[3] = { 0.074f, 0.254f, 0.745f };
	float color_BoundingBox[3] = { 0.074f, 0.254f, 0.745f };
	float color_Chams[3] = { 0.074f, 0.254f, 0.745f };
	float color_ChamsWall[3] = { 0.074f, 0.254f, 0.745f };
	float color_FakeAngleGhost[3] = { 1.f, 1.f, 1.f };
	float color_Skeletons[3] = { 1.f, 1.f, 1.f };
	float color_Bullettracer[3] = { 0.611f, 0.050f, 0.627f };
	float color_Glow[3] = { 0.074f, 0.254f, 0.745f };
	float color_SpreadCircle[3] = { 0.074f, 0.254f, 0.745f };
	float color_DamageIndicator[3] = { 1.f, 1.f, 1.f };
	float color_Ammobar[3] = { 0.611f, 0.050f, 0.627f };
	float color_entBox[3] = { 0.074f, 0.254f, 0.745f };
	float color_entName[3] = { 0.074f, 0.254f, 0.745f };
	float color_entGlow[3] = { 0.074f, 0.254f, 0.745f };
	float color_ResolverInfo[3] = { 1.f, 1.f, 1.f };
	float color_BotInfo[3] = { 1.f, 1.f, 1.f };
	float color_OffScreen[3] = { 0.074f, 0.254f, 0.745f };
};

extern CSettings* g_Settings;
