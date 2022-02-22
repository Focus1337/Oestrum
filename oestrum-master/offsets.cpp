#include "sdk.h"
#include <Psapi.h>
#include "xor.h"
#include "global.h"
#include "hooks.h"

offsets_t offs;

uint64_t FindPatternIDA(const char* szModule, const char* szSignature)
{
#define INRANGE(x,a,b)  (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),XorStr('A'),XorStr('F')) ? ((x&(~0x20)) - XorStr('A') + 0xa) : (INRANGE(x,XorStr('0'),XorStr('9')) ? x - XorStr('0') : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &modInfo, sizeof(MODULEINFO));
	DWORD startAddress = (DWORD)modInfo.lpBaseOfDll;
	DWORD endAddress = startAddress + modInfo.SizeOfImage;
	const char* pat = szSignature;
	DWORD firstMatch = 0;
	for (DWORD pCur = startAddress; pCur < endAddress; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == XorStr('\?') || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == XorStr('\?\?') || *(PBYTE)pat != XorStr('\?')) pat += 3;
			else pat += 2;    //one ?
		}
		else {
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}

static bool bCompare(const BYTE* Data, const BYTE* Mask, const char* szMask)
{
	for (; *szMask; ++szMask, ++Mask, ++Data)
	{
		if (*szMask == 'x' && *Mask != *Data)
		{
			return false;
		}
	}
	return (*szMask) == 0;
}

DWORD WaitOnModuleHandle(std::string moduleName)
{
	DWORD ModuleHandle = NULL;
	while (!ModuleHandle)
	{
		ModuleHandle = (DWORD)GetModuleHandle(moduleName.c_str());
		if (!ModuleHandle)
			Sleep(50);
	}
	return ModuleHandle;
}

DWORD FindPattern(std::string moduleName, BYTE* Mask, char* szMask)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;
	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), Mask, szMask))
		{
			return DWORD(Address + c);
		}
	}
	return 0;
}


void SetupOffsets()
{
	printf(XorStr(":::::Adresses & Offsets:::::\n"));
	g_pNetVars = new CNetVars();

	offs.getSequenceActivity = (DWORD)FindPatternIDA(XorStr("client.dll"), XorStr("55 8B EC 83 7D 08 FF 56 8B F1 74 3D"));
	offs.invalidateBoneCache = (DWORD)FindPatternIDA(XorStr("client.dll"), XorStr("80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81"));
	offs.bOverridePostProcessingDisable = *(DWORD*)(FindPatternIDA(XorStr("client.dll"), XorStr("80 3D ? ? ? ? ? 53 56 57 0F 85")) + 2);

	offs.dwInitKeyValues = FindPatternIDA(XorStr("client.dll"), XorStr("8B 0E 33 4D FC 81 E1 ? ? ? ? 31 0E 88 46 03 C1 F8 08 66 89 46 12 8B C6")) - 0x45;
	printf(XorStr("InitKeyValues: 0x%X\n"), (DWORD)offs.dwInitKeyValues);

	offs.dwLoadFromBuffer = FindPatternIDA(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04"));
	printf(XorStr("LoadFromBuffer: 0x%X\n"), (DWORD)offs.dwLoadFromBuffer);

	offs.m_ArmorValue = g_pNetVars->GetOffset("DT_CSPlayer", "m_ArmorValue");
	offs.m_bHasHelmet = g_pNetVars->GetOffset("DT_CSPlayer", "m_bHasHelmet");
	offs.m_iTeamNum = g_pNetVars->GetOffset("DT_CSPlayer", "m_iTeamNum");
	offs.m_angRotation = g_pNetVars->GetOffset("DT_CSPlayer", "m_angRotation");
	offs.m_lifeState = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_lifeState"));
	offs.m_flPoseParameter = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flPoseParameter"));
	offs.m_flCycle = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flCycle"));
	offs.m_flSimulationTime = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flSimulationTime"));
	offs.m_nSequence = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_nSequence"));
	offs.m_flLowerBodyYawTarget = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flLowerBodyYawTarget"));
	offs.m_angEyeAngles = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_angEyeAngles[0]"));
	offs.m_nTickBase = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	offs.m_fFlags = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_fFlags"));
	offs.m_flNextPrimaryAttack = g_pNetVars->GetOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_flNextPrimaryAttack"));
	offs.m_iClip1 = g_pNetVars->GetOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_iClip1"));
	offs.m_vecOrigin = g_pNetVars->GetOffset(XorStr("DT_BaseEntity"), XorStr("m_vecOrigin"));
	offs.m_vecViewOffset = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_vecViewOffset[0]"));
	offs.m_aimPunchAngle = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_aimPunchAngle"));
	offs.m_aimPunchAngleVel = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_aimPunchAngleVel"));
	offs.m_vecVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_vecVelocity[0]");
	offs.m_vecBaseVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_vecBaseVelocity");
	offs.m_flFallVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFallVelocity");
	offs.m_bPinPulled = g_pNetVars->GetOffset(XorStr("DT_BaseCSGrenade"), XorStr("m_bPinPulled"));
	offs.m_fThrowTime = g_pNetVars->GetOffset(XorStr("DT_BaseCSGrenade"), XorStr("m_fThrowTime"));
	offs.m_fAccuracyPenalty = g_pNetVars->GetOffset("DT_WeaponCSBase", "m_fAccuracyPenalty");
	offs.m_flC4Blow = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_flC4Blow"));
	offs.m_bBombDefused = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_bBombDefused"));
	offs.m_hOwnerEntity = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_hOwnerEntity"));
	offs.m_flFriction = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFriction");
	offs.m_CollisionGroup = g_pNetVars->GetOffset("DT_BaseEntity", "m_CollisionGroup");
	offs.m_bIsScoped = g_pNetVars->GetOffset("DT_CSPlayer", "m_bIsScoped");
	offs.m_flFlashDuration = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFlashDuration");
	offs.m_hObserverTarget = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_hObserverTarget"));
	offs.m_hMyWearables = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_hMyWearables"));
	offs.m_hMyWeapons = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_hMyWeapons"));
	offs.m_hActiveWeapon = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_hActiveWeapon"));
	offs.m_flFriction = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFriction");
	offs.m_flMaxspeed = g_pNetVars->GetOffset("DT_BasePlayer", "m_flMaxspeed");
	offs.m_flStepSize = g_pNetVars->GetOffset("DT_CSPlayer", "m_flStepSize");
	offs.m_bGunGameImmunity = g_pNetVars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
	offs.m_flPostponeFireReadyTime = g_pNetVars->GetOffset("DT_WeaponCSBaseGun", "m_flPostponeFireReadyTime");
	offs.m_iShotsFired = g_pNetVars->GetOffset("DT_CSPlayer", "m_iShotsFired");

	/*DT_BaseAttributableItem*/
	offs.m_iItemDefinitionIndex = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	offs.m_iItemIDHigh = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemIDHigh");
	offs.m_iItemIDLow = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemIDLow");
	offs.m_iEntityQuality = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iEntityQuality");
	offs.m_szCustomName = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_szCustomName");
	offs.m_OriginalOwnerXuidLow = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	offs.m_OriginalOwnerXuidHigh = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
	offs.m_nFallbackPaintKit = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackPaintKit");
	offs.m_nFallbackSeed = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackSeed");
	offs.m_flFallbackWear = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_flFallbackWear");
	offs.m_nFallbackStatTrak = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackStatTrak");
	offs.m_nAccountID = g_pNetVars->GetOffset("DT_FEPlayerDecal", "m_unAccountID");
	offs.m_iViewModelIndex = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_iViewModelIndex");
	offs.m_iWorldModelIndex = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_iWorldModelIndex");
	offs.m_hWeaponWorldModel = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_hWeaponWorldModel");
	offs.m_nModeIndex = g_pNetVars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
	offs.m_hViewModel = g_pNetVars->GetOffset("DT_CSPlayer", "m_hViewModel[0]");
	offs.m_hWeapon = g_pNetVars->GetOffset("DT_BaseViewModel", "m_hWeapon");
}