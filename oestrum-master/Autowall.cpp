#include "Autowall.h"
#include "global.h"
#include "Math.h"
#include "hooks.h"
#include "GameUtils.h"

#define DAMAGE_NO			0
#define DAMAGE_EVENTS_ONLY	1	
#define DAMAGE_YES			2
#define DAMAGE_AIM			3

#pragma region Utils
bool CAutowall::DidHitNonWorldEntity(CBaseEntity* m_pEnt)
{
	return m_pEnt != NULL && m_pEnt != g_pEntitylist->GetClientEntity(0);
}

bool CAutowall::IsWorldEntity(CBaseEntity* m_pEnt)
{
	return m_pEnt != NULL && m_pEnt->GetIndex() == 0;
}

bool CAutowall::isBreakableEntity(CBaseEntity *pEntity)
{
	typedef bool(__thiscall *isBreakbaleEntityFn)(CBaseEntity*);
	static isBreakbaleEntityFn IsBreakableEntity = (isBreakbaleEntityFn)FindPatternIDA(XorStr("client.dll"), XorStr("55 8B EC 51 56 8B F1 85 F6 74 68 83 BE"));


	if (pEntity == NULL)
		return false;

	if (pEntity->GetIndex() == 0)
		return false;

	auto takeDamage{ (char *)((uintptr_t)pEntity + *(size_t *)((uintptr_t)IsBreakableEntity + 38)) };
	auto takeDamageBackup{ *takeDamage };

	ClientClass *pClass = g_pClient->GetAllClasses();

	if ((pClass->m_pNetworkName[1]) != 'F'
		|| (pClass->m_pNetworkName[4]) != 'c'
		|| (pClass->m_pNetworkName[5]) != 'B'
		|| (pClass->m_pNetworkName[9]) != 'h')
		*takeDamage = DAMAGE_YES;

	bool breakable = IsBreakableEntity(pEntity);
	*takeDamage = takeDamageBackup;

	return breakable;
}

void CAutowall::ClipTraceToPlayers(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, ITraceFilter *filter, CGameTrace *tr)
{
	static DWORD ClipTraceToPlayersAdd = (DWORD)FindPatternIDA(XorStr("client.dll"), XorStr("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10"));

	_asm
	{
		mov eax, filter
		lea ecx, tr
		push ecx
		push eax
		push mask
		lea edx, vecAbsEnd
		lea ecx, vecAbsStart
		call ClipTraceToPlayersAdd
		add esp, 0xC
	}
}

void CAutowall::TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, CBaseEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_pEngineTrace->TraceRay_NEWEST(ray, mask, &filter, ptr);
}


bool CAutowall::TraceToExit(CGameTrace *enterTrace, Vector vecStartPosition, Vector vecDir, CGameTrace *exitTrace)
{
	Vector vecEnd;
	float flDistance = 0.f;
	signed int iDistanceCheck = 23;
	int iFirstContents = 0;

	do // We want to call the code once before checking for distance
	{
		flDistance += 4.f;
		vecEnd = vecStartPosition + vecDir * flDistance;

		if (!iFirstContents)
			iFirstContents = g_pEngineTrace->GetPointContents(vecEnd, MASK_SHOT | CONTENTS_GRATE, NULL); /*0x4600400B*/
		int iPointContents = g_pEngineTrace->GetPointContents(vecEnd, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(iPointContents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || iPointContents & CONTENTS_HITBOX && iPointContents != iFirstContents) /*0x600400B, *0x40000000*/
		{
			Vector vecNewEnd = vecEnd - (vecDir * 4.f);

			Ray_t ray;
			ray.Init(vecEnd, vecNewEnd);

			g_pEngineTrace->TraceRay_NEWEST(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exitTrace);

			if (exitTrace->startsolid && exitTrace->surface.flags & SURF_HITBOX) //A Hitbox is infront of the enemy and the enemy is behind a solid wall
			{
				TraceLine(vecEnd, vecStartPosition, MASK_SHOT_HULL | CONTENTS_HITBOX, exitTrace->m_pEnt, exitTrace);

				if (exitTrace->DidHit() && !exitTrace->startsolid) //DidHit() && !startsolid
					return true;
				continue;
			}

			if (exitTrace->DidHit() && !exitTrace->startsolid)
			{
				if (enterTrace->surface.flags & SURF_NODRAW || !(exitTrace->surface.flags & SURF_NODRAW))
				{
					if (exitTrace->plane.normal.Dot(vecDir) <= 1.f)
						return true;
					continue;
				}

				//For some reason, Skeet's pesudo claims this is || but if you use ||, you will spam windowsils. I would have to look into this.
				if (isBreakableEntity(enterTrace->m_pEnt) && isBreakableEntity(exitTrace->m_pEnt))
					return true;
				continue;
			}

			if (exitTrace->surface.flags & SURF_NODRAW)
			{
				if (isBreakableEntity(enterTrace->m_pEnt) && isBreakableEntity(exitTrace->m_pEnt))
					return true;
				else if (!(enterTrace->surface.flags & SURF_NODRAW))
					continue;
			}

			if (IsWorldEntity(enterTrace->m_pEnt) && (isBreakableEntity(enterTrace->m_pEnt)))
			{
				exitTrace = enterTrace;
				exitTrace->endpos = vecStartPosition + vecDir;
				return true;
			}
			continue;
		}
		--iDistanceCheck;
	} while (iDistanceCheck); //Distance check

	return false;
}
#pragma endregion

bool CAutowall::IsArmored(CBaseEntity* Entity, int ArmorValue, int Hitgroup)
{
	CBaseEntity* Player = (CBaseEntity*)Entity;

	if (!Player)
		return false;

	bool result = false;

	if (ArmorValue > 0)
	{
		switch (Hitgroup)
		{
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			result = true;
			break;
		case HITGROUP_HEAD:
			result = Player->HasHelmet(); // DT_CSPlayer -> m_bHasHelmet
			break;
		}
	}

	return result;
}

bool CAutowall::HandleBulletPenetration(CSWeaponInfo *wpn_data, FireBulletData &data)
{
	CGameTrace trace_exit;
	surfacedata_t *enterSurfaceData = g_pPhysics->GetSurfaceData(data.enterTrace.surface.surfaceProps);
	int iEnterMaterial = enterSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float flFinalDamageModifier = 0.16f;
	float flCombinedPenetrationModifier = 0.f;
	bool isSolidSurf = ((data.enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((data.enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (data.penetrateCount <= 0
		|| (!data.penetrateCount && !isLightSurf && !isSolidSurf && iEnterMaterial != CHAR_TEX_GLASS && iEnterMaterial != CHAR_TEX_GRATE)
		|| wpn_data->flPenetration <= 0.f
		|| !TraceToExit(&data.enterTrace, data.enterTrace.endpos, data.direction, &trace_exit)
		&& !(g_pEngineTrace->GetPointContents(data.enterTrace.endpos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX)))
		return false;

	surfacedata_t *exitSurfaceData = g_pPhysics->GetSurfaceData(trace_exit.surface.surfaceProps);
	int iExitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;

	if (iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE)
	{
		flCombinedPenetrationModifier = 3.f;
		flFinalDamageModifier = 0.05f;
	}

	else if (isLightSurf || isSolidSurf)
	{
		flCombinedPenetrationModifier = 1.f;
		flFinalDamageModifier = 0.16f;
	}
	else
	{
		flCombinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) * 0.5f;
		flFinalDamageModifier = 0.16f;
	}

	if (iEnterMaterial == iExitMaterial)
	{
		if (iExitMaterial == CHAR_TEX_CARDBOARD || iExitMaterial == CHAR_TEX_WOOD)
			flCombinedPenetrationModifier = 3.f;
		else if (iExitMaterial == CHAR_TEX_PLASTIC)
			flCombinedPenetrationModifier = 2.0f;
	}

	float flThickness = (trace_exit.endpos - data.enterTrace.endpos).LengthSqr();
	float flModifier = fmaxf(0.f, 1.f / flCombinedPenetrationModifier);

	float flLostDamage = fmaxf(
		((flModifier * flThickness) / 24.f) //* 0.041666668
		+ ((data.currentDamage * flFinalDamageModifier)
			+ (fmaxf(3.75 / wpn_data->flPenetration, 0.f) * 3.f * flModifier)), 0.f);

	if (flLostDamage > data.currentDamage)
		return false;

	if (flLostDamage > 0.f)
		data.currentDamage -= flLostDamage;

	if (data.currentDamage < 1.f)
		return false;

	data.src = trace_exit.endpos;
	data.penetrateCount--;

	return true;
}

bool CAutowall::SimulateFireBullet(CBaseCombatWeapon* pWeapon, FireBulletData &data, bool runOnce)
{
	if (pWeapon == NULL)
		return false;

	CGameTrace exitTrace;
	CBaseEntity* pLocalEntity = (CBaseEntity*)Global::LocalPlayer;
	CSWeaponInfo* weaponData = pWeapon->GetCSWpnData();
	surfacedata_t *enterSurfaceData = g_pPhysics->GetSurfaceData(data.enterTrace.surface.surfaceProps);
	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

	data.penetrateCount = 4;
	data.traceLength = 0.0f;

	if (weaponData == NULL)
		return false;

	data.currentDamage = (float)weaponData->iDamage;

	if (data.penetrateCount > 0)
	{
		while (data.currentDamage > 1.f)
		{
			data.traceLengthRemaining = weaponData->flRange - data.traceLength;

			Vector end = (data.direction * data.traceLengthRemaining) + data.src;

			TraceLine(data.src, end, MASK_SHOT | CONTENTS_GRATE, pLocalEntity, &data.enterTrace);

			ClipTraceToPlayers(data.src, end + data.direction * 40.f, MASK_SHOT | CONTENTS_GRATE, &data.filter, &data.enterTrace);

			if (data.enterTrace.fraction == 1.0f)
				break;

			data.traceLength += data.enterTrace.fraction * data.traceLengthRemaining;
			data.currentDamage *= weaponData->flRangeModifier; //pow's and random numbers are for the weak, fuck off, 0.002


			ClientClass* pClass = (ClientClass*)pLocalEntity->GetClientClass();

			if (((data.enterTrace.hitgroup <= HITGROUP_RIGHTLEG) && (data.enterTrace.hitgroup > HITGROUP_GENERIC) && (CBaseEntity*)pLocalEntity && pClass->m_ClassID == ClassId_CCSPlayer && ((CBaseEntity*)pLocalEntity)->GetTeamNum() != ((CBaseEntity*)data.enterTrace.m_pEnt)->GetTeamNum()) && (((CBaseEntity*)data.enterTrace.m_pEnt)->GetTeamNum() == 2 || ((CBaseEntity*)data.enterTrace.m_pEnt)->GetTeamNum() == 3))
			{
				bool HeavyArmor = ((CBaseEntity*)data.enterTrace.m_pEnt)->m_bHasHeavyArmor(); // DT_CSPlayer -> m_bHasHeavyArmor
				int ArmorValue = ((CBaseEntity*)data.enterTrace.m_pEnt)->GetArmor(); // DT_CSPlayer -> m_ArmorValue

				auto iHitGroup = data.enterTrace.hitgroup;

				switch (iHitGroup) //scaledamage
				{
				case HITGROUP_HEAD:
					if (HeavyArmor)//If the enemy is using Heavy Armor, headshots will do half damage.
						data.currentDamage *= 4.f * 0.5f;
					else
						data.currentDamage *= 4.f;
					break;
				case HITGROUP_STOMACH:
					data.currentDamage *= 1.25f;
					break;
				case HITGROUP_LEFTLEG:
				case HITGROUP_RIGHTLEG:
					data.currentDamage *= 0.75f;
					break;
				}

				if (IsArmored(data.enterTrace.m_pEnt, ArmorValue, data.enterTrace.hitgroup))
				{
					float flBonusValue = 1.f, flArmorBonusRatio = 0.5f, flArmorRatio = weaponData->flArmorRatio * 0.5f;

					if (HeavyArmor)
					{
						flArmorBonusRatio = 0.33f;
						flArmorRatio *= 0.5f;
						flBonusValue = 0.33f;
					}

					auto NewDamage = data.currentDamage * flArmorRatio;

					if (HeavyArmor)
						NewDamage *= 0.85f;

					if (((data.currentDamage - (data.currentDamage * flArmorRatio)) * (flBonusValue * flArmorBonusRatio)) > ArmorValue)
						NewDamage = data.currentDamage - (ArmorValue / flArmorBonusRatio);

					//Thanks esoterik <c>
					if (NewDamage > 115)
						NewDamage = 115;

					data.currentDamage = NewDamage;
				}
				return true;
			}

			if ((data.traceLength > 3000.f) || (enterSurfPenetrationModifier < 0.1f) || !HandleBulletPenetration(weaponData, data))
				break;

			if (runOnce)
				return true;
		}
	}
	return false;
}

float CAutowall::CanHit(Vector &vecEyePos, Vector &point)
{
	FireBulletData data;
	data.src = vecEyePos;
	data.filter.pSkip = Global::LocalPlayer;

	Vector angles;
	Vector tmp = point - data.src;

	Math::VectorAngles(tmp, angles);
	Math::AngleVectors(angles, &data.direction);
	data.direction.NormalizeInPlace();

	if (SimulateFireBullet(Global::LocalPlayer->GetWeapon(), data, false))
		return data.currentDamage;
	return -1;
}

float CAutowall::GetWallDamage(Vector &vecEyePos, QAngle angles)
{
	FireBulletData data;
	data.src = vecEyePos;
	data.filter.pSkip = Global::LocalPlayer;

	Math::AngleVectors(Vector(angles.x, angles.y, angles.z), &data.direction);
	data.direction.NormalizeInPlace();

	if (!Global::LocalPlayer->GetWeapon())
		return -1;

	if (SimulateFireBullet(Global::LocalPlayer->GetWeapon(), data, true))
		return data.currentDamage;

	return -1;
}

CAutowall* g_Autowall = new CAutowall();