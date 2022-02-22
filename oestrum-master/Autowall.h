#pragma once
#include "sdk.h"

struct FireBulletData
{
	Vector          src;
	CGameTrace      enterTrace;
	Vector          direction;
	CTraceFilter    filter;
	float           traceLength;
	float           traceLengthRemaining;
	float           currentDamage;
	int             penetrateCount;
};

class CAutowall
{
public:
	float CanHit(Vector& vecEyePos, Vector& point);
	float GetWallDamage(Vector& vecEyePos, QAngle angles);
private:
	bool DidHitNonWorldEntity(CBaseEntity* m_pEnt);
	bool IsWorldEntity(CBaseEntity* m_pEnt);
	bool isBreakableEntity(CBaseEntity* pEntity);
	void ClipTraceToPlayers(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr);
	void TraceLine(Vector & vecAbsStart, Vector & vecAbsEnd, unsigned int mask, CBaseEntity * ignore, CGameTrace * ptr);
	bool IsArmored(CBaseEntity* Entity, int ArmorValue, int Hitgroup);
	bool TraceToExit(CGameTrace* enterTrace, Vector vecStartPosition, Vector vecDir, CGameTrace* exitTrace);
	bool HandleBulletPenetration(CSWeaponInfo* wpn_data, FireBulletData& data);
	bool SimulateFireBullet(CBaseCombatWeapon* pWeapon, FireBulletData& data, bool runOnce);
};

extern CAutowall* g_Autowall;