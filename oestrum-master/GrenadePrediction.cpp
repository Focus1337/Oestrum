#include "GrenadePrediction.h"
#include "SDK.h"
#include "sdk.h"
#include <assert.h>
#include "global.h"
#include "GameUtils.h"
#include "Draw.h"
#include "Math.h"

void grenade_prediction::Tick(int buttons)
{
	if (!g_Settings->visuals_GrenadePrediction)
		return;

	bool in_attack = (buttons & IN_ATTACK);
	bool in_attack2 = (buttons & IN_ATTACK2);

	act = (in_attack && in_attack2) ? ACT_DROP :
		(in_attack2) ? ACT_THROW :
		(in_attack) ? ACT_LOB :
		ACT_NONE;
}

void grenade_prediction::View()
{
	if (!g_Settings->visuals_GrenadePrediction)
		return;

	if (!Global::LocalPlayer)
		return;

	auto weapon = Global::MainWeapon;//Global::LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	if ((weapon->IsGrenade()) && act != ACT_NONE)
	{
		QAngle Angles;
		g_pEngine->GetViewAngles(Angles);

		ClientClass* pWeaponClass = (ClientClass*)weapon->GetClientClass();
		if (!pWeaponClass)
		{
			type = -1;
			Simulate(Angles, Global::LocalPlayer);
		}
		else
		{
			type = pWeaponClass->m_ClassID;
			Simulate(Angles, Global::LocalPlayer);
		}
	}
	else
	{
		type = -1;
	}
}

inline float CSGO_Armor(float flDamage, int ArmorValue) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast< float >(ArmorValue)) {
			flArmor = static_cast< float >(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

void grenade_prediction::Paint()
{
	if (!g_Settings->visuals_GrenadePrediction)
		return;

	if (!Global::LocalPlayer)
		return;

	auto weapon = Global::MainWeapon;//Global::LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	if ((type) && path.size()>1 && act != ACT_NONE && weapon->IsGrenade())
	{
		Vector ab, cd;
		Vector prev = path[0];
		for (auto it = path.begin(), end = path.end(); it != end; ++it)
		{
			if (GameUtils::WorldToScreen(prev, ab) && GameUtils::WorldToScreen(*it, cd))
			{
				g_pSurface->SetDrawColor(TracerColor);
				g_pSurface->DrawLine(ab[0], ab[1], cd[0], cd[1]);
			}
			prev = *it;
		}

		for (auto it = OtherCollisions.begin(), end = OtherCollisions.end(); it != end; ++it)
		{
			g_Draw.Draw3DCube(2.f, it->second, it->first, Color(0, 255, 0, 200));
		}

		g_Draw.Draw3DCube(2.f, OtherCollisions.rbegin()->second, OtherCollisions.rbegin()->first, Color(255, 0, 0, 200));

		std::string EntName;
		auto bestdmg = 0;
		static Color redcol = { 255, 0, 0, 255 };
		static Color greencol = { 25, 255, 25, 255 };
		static Color yellowgreencol = { 177, 253, 2, 255 };
		static Color yellowcol = { 255, 255, 0, 255 };
		static Color orangecol = { 255, 128, 0, 255 };
		Color *BestColor = &redcol;

		Vector endpos = path[path.size() - 1];
		Vector absendpos = endpos;

		float totaladded = 0.0f;

		while (totaladded < 30.0f) {
			if (g_pEngineTrace->GetPointContents(endpos) == CONTENTS_EMPTY)
				break;

			totaladded += 2.0f;
			endpos.z += 2.0f;
		}

		//CBaseCombatWeapon* pWeapon = Global::LocalPlayer->m_hActiveWeapon().Get();
		CBaseCombatWeapon* pWeapon = Global::MainWeapon;
		int weap_id = pWeapon->GetItemDefinitionIndex();

		if (pWeapon &&
			weap_id == HEGRENADE ||
			weap_id == MOLOTOV ||
			weap_id == INCGRENADE) {
			for (int i = 1; i < 64; i++) {
				CBaseEntity *pEntity = (CBaseEntity*)g_pEntitylist->GetClientEntity(i);

				if (!pEntity || pEntity->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
					continue;

				player_info_s info;
				g_pEngine->GetPlayerInfo(pEntity->GetIndex(), &info);

				float dist = (pEntity->GetOrigin() - endpos).Length();

				if (dist < 350.0f) {
					CTraceFilter filter;
					filter.pSkip = Global::LocalPlayer;
					Ray_t ray;
					Vector NadeScreen;
					GameUtils::WorldToScreen(endpos, NadeScreen);

					Vector vPelvis = pEntity->GetBonePos(HITBOX_PELVIS);
					ray.Init(endpos, vPelvis);
					trace_t ptr;
					g_pEngineTrace->TraceRay(ray, MASK_SHOT, &filter, &ptr);

					if (ptr.m_pEnt == pEntity) {
						Vector PelvisScreen;

						GameUtils::WorldToScreen(vPelvis, PelvisScreen);

						static float a = 105.0f;
						static float b = 25.0f;
						static float c = 140.0f;

						float d = ((((pEntity->GetOrigin()) - prev).Length() - b) / c);
						float flDamage = a*exp(-d * d);
						auto dmg = max(static_cast<int>(ceilf(CSGO_Armor(flDamage, pEntity->GetArmor()))), 0);

						Color *destcolor = dmg >= 65 ? &redcol : dmg >= 40.0f ? &orangecol : dmg >= 20.0f ? &yellowgreencol : &greencol;

						if (dmg > bestdmg) {
							EntName = info.m_szPlayerName;//pEntity->GetName();
							BestColor = destcolor;
							bestdmg = dmg;
						}
					}
				}
			}
		}

		if (bestdmg > 0.f) {
			if (weap_id != HEGRENADE)
			{
				if (GameUtils::WorldToScreen(prev, cd))
					g_Draw.StringB(g_Draw.font_esp2, cd[0], cd[1] - 10, *BestColor, FONT_CENTER, firegrenade_didnt_hit ? "No collisions" : (EntName + " will be burnt.").c_str());
			}
			else
			{
				if (GameUtils::WorldToScreen(*path.begin(), cd))
					g_Draw.StringB(g_Draw.font_esp2, cd[0], cd[1] - 10, *BestColor, FONT_CENTER, ("Most damage dealt to: " + EntName + " -" + std::to_string(bestdmg)).c_str());
			}
		}
	}
}

void grenade_prediction::Setup(CBaseEntity* pl, Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles)
{
	QAngle angThrow = angEyeAngles;
	float pitch = angThrow.x;

	if (pitch <= 90.0f)
	{
		if (pitch<-90.0f)
		{
			pitch += 360.0f;
		}
	}
	else
	{
		pitch -= 360.0f;
	}
	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	// Gets ThrowVelocity from weapon files
	// Clamped to [15,750]
	float flVel = 750.0f * 0.9f;

	// Do magic on member of grenade object [esi+9E4h]
	// m1=1  m1+m2=0.5  m2=0
	static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	float b = power[act];
	// Clamped to [0,1]
	b = b * 0.7f;
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	Math::AngleVectors(angThrow, vForward, vRight, vUp);

	vecSrc = pl->GetOrigin();
	vecSrc += pl->ViewOffset();
	float off = (power[act] * 12.0f) - 12.0f;
	vecSrc.z += off;

	// Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest.MulAdd(vecDest, vForward, 22.0f);
	TraceHull(vecSrc, vecDest, tr);

	// After the hull trace it moves 6 units back along vForward
	// vecSrc = tr.endpos - vForward * 6
	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	// Finally calculate velocity
	vecThrow = pl->GetVelocity(); vecThrow *= 1.25f;
	vecThrow.MulAdd(vecThrow, vForward, flVel);
}

void grenade_prediction::Simulate(QAngle& Angles, CBaseEntity* pLocal)
{
	Vector vecSrc, vecThrow;
	Setup(pLocal, vecSrc, vecThrow, Angles);

	float interval = g_pGlobals->interval_per_tick;

	// Log positions 20 times per sec
	int logstep = static_cast<int>(0.05f / interval);
	int logtimer = 0;

	path.clear();
	OtherCollisions.clear();
	TracerColor = Color(255, 255, 0, 255);
	for (unsigned int i = 0; i<path.max_size() - 1; ++i)
	{
		if (!logtimer)
			path.push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval);
		if ((s & 1) || vecThrow == Vector(0, 0, 0))
			break;

		// Reset the log timer every logstep OR we bounced
		if ((s & 2) || logtimer >= logstep) logtimer = 0;
		else ++logtimer;
	}
	path.push_back(vecSrc);
}

int grenade_prediction::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{
	// Apply gravity
	Vector move;
	AddGravityMove(move, vecThrow, interval, false);

	// Push entity
	trace_t tr;
	PushEntity(vecSrc, move, tr);

	int result = 0;
	// Check ending conditions
	if (CheckDetonate(vecThrow, tr, tick, interval))
	{
		result |= 1;
	}

	// Resolve collisions
	if (tr.fraction != 1.0f)
	{
		result |= 2; // Collision!
		ResolveFlyCollisionCustom(tr, vecThrow, interval);
	}

	if (tr.m_pEnt && ((CBaseEntity*)tr.m_pEnt)->IsPlayer())
	{
		TracerColor = Color(255, 0, 0, 255);
	}

	if ((result & 1) || vecThrow == Vector(0, 0, 0) || tr.fraction != 1.0f)
	{
		QAngle angles;
		Math::VectorAngles((tr.endpos - tr.start).Normalized(), angles);
		OtherCollisions.push_back(std::make_pair(tr.endpos, angles));
	}

	// Set new position
	vecSrc = tr.endpos;

	return result;
}

bool grenade_prediction::CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	firegrenade_didnt_hit = false;
	switch (type)
	{
	case ClassId_CSmokeGrenade:
	case ClassId_CDecoyGrenade:
		// Velocity must be <0.1, this is only checked every 0.2s
		if (vecThrow.Length()<0.1f)
		{
			int det_tick_mod = static_cast<int>(0.2f / interval);
			return !(tick%det_tick_mod);
		}
		return false;

		/* TIMES AREN'T COMPLETELY RIGHT FROM WHAT I'VE SEEN ! ! ! */
	case ClassId_CMolotovGrenade:
	case ClassId_CIncendiaryGrenade:
		// Detonate when hitting the floor
		if (tr.fraction != 1.0f && tr.plane.normal.z>0.7f)
			return true;
		// OR we've been flying for too long

	case ClassId_CFlashbang:
	case ClassId_CHEGrenade: {
		// Pure timer based, detonate at 1.5s, checked every 0.2s
		firegrenade_didnt_hit = static_cast<float>(tick)*interval > 1.5f && !(tick%static_cast<int>(0.2f / interval));
		return firegrenade_didnt_hit;
	}
	default:
		Assert(false);
		return false;
	}
}

void grenade_prediction::TraceHull(Vector& src, Vector& end, trace_t& tr)
{
	// Setup grenade hull
	static const Vector hull[2] = { Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f) };

	CTraceFilter filter;
	filter.SetIgnoreClass("BaseCSGrenadeProjectile");
	filter.pSkip = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

	Ray_t ray;
	ray.Init(src, end, hull[0], hull[1]);

	const unsigned int mask = 0x200400B;
	g_pEngineTrace->TraceRay(ray, mask, &filter, &tr);
}

void grenade_prediction::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	Vector basevel(0.0f, 0.0f, 0.0f);

	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
	{
		move.z = (vel.z + basevel.z) * frametime;
	}
	else
	{
		// Game calls GetActualGravity( this );
		float gravity = 800.0f * 0.4f;

		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

		vel.z = newZ;
	}
}

void grenade_prediction::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;

	// Trace through world
	TraceHull(src, vecAbsEnd, tr);
}

void grenade_prediction::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval)
{
	// Calculate elasticity
	float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
	float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity>0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity<0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	// Stop completely once we move too slow
	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
	if (flSpeedSqr<flMinSpeedSqr)
	{
		vecAbsVelocity.Zero();
	}

	// Stop if on ground
	if (tr.plane.normal.z>0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity.Mul((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}
}

int grenade_prediction::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float    backoff;
	float    change;
	float    angle;
	int        i, blocked;

	blocked = 0;

	angle = normal[2];

	if (angle > 0)
	{
		blocked |= 1;        // floor
	}
	if (!angle)
	{
		blocked |= 2;        // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i<3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
		{
			out[i] = 0;
		}
	}

	return blocked;
}