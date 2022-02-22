#pragma once
#include <vector>
#include "sdk.h"

#define SIZEOF(var) ( sizeof( var ) / sizeof( var[0] ))

class itemTimer {
public:
	itemTimer();
	itemTimer(float maxTime);
	float getTimeRemaining();
	float getTimeRemainingRatio();
	float getMaxTime();
	void setMaxTime(float);
	void startTimer();
private:
	float timeStarted;
	float maxTime;
};

class FloatingText
{
public:
	void Draw();
	float TimeCreated;
	float ExpireTime;
	FloatingText(CBaseEntity* attachEnt, float lifetime, int Damage);

private:
	CBaseEntity* pEnt;
	int DamageAmt;

};

class CScreen
{
public:
	int width, height;
};

class CGlobalPointers
{
public:
	CBaseEntity* LocalPlayer;
	CBaseCombatWeapon* MainWeapon;
	CUserCmd*	UserCmd;
}; extern CGlobalPointers* g_GlobalPointers;


namespace Global
{
	extern QAngle							RealAngle;
	extern QAngle							FakeAngle;
	extern QAngle							LastAngle;
	extern QAngle							StrafeAngle;
	extern QAngle							AAAngle;
	extern CBaseEntity*						LocalPlayer;
	extern CBaseCombatWeapon*				MainWeapon;
	extern CUserCmd*						pCmd;
	extern CSWeaponInfo*					WeaponData;
	extern Vector							vecUnpredictedVel;
	extern HWND								Window;
	extern CScreen							Screen;
	extern std::vector<FloatingText>		DamageHit;
	extern std::vector<Vector>				walkpoints;
	extern bool								ForceRealAA;
	extern bool								Return;
	extern bool								SendPacket;
	extern bool								ShowMenu;
	extern bool								Opened;
	extern bool								Init;
	extern bool								bShouldChoke;
	extern float							CurrTime;
	extern int								ChokedPackets;
	extern int								DamageDealt;
	extern int								nChockedTicks;
}

class CDataMapUtils {
public:
	int Find(datamap_t *pMap, const char* szName);
};


extern CDataMapUtils* g_pData;

extern CBaseEntity* Global::LocalPlayer;
extern CBaseCombatWeapon* Global::MainWeapon;
extern CUserCmd*	Global::pCmd;
