#pragma once
#include "sdk.h"

#define TICK_INTERVAL			(g_pGlobals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )


class CAimbot
{

public:
	void Run(CUserCmd * cmd);
	void RemoveRecoil(CUserCmd * cmd);
	Vector GetHitboxPos(CBaseEntity* entity, int hitbox_id);
	bool fired;
	float bestdist = 8192.f;
private:
	float HitChance(CBaseCombatWeapon * weapon);
	Vector ScanPoint(CBaseEntity * entity);
	bool CanShoot();
	bool GoodBacktrackTick(int tick);
	void LbyBacktrack(CUserCmd * pCmd, CBaseEntity * pLocal, CBaseEntity * pEntity);
	int ScanHitbox(CBaseEntity * entity);
	mstudiobbox_t* GetHitbox(CBaseEntity* entity, int hitbox_index);
};

extern CAimbot* g_Aimbot;