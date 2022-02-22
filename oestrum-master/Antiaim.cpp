#include "sdk.h"
#include "Antiaim.h"
#include "global.h"
#include "GameUtils.h"
#include "Math.h"
#include "INetChannel.h"
#include "Aimbot.h"

#pragma region Freestand stuff
float fov2player(Vector ViewOffSet, Vector View, CBaseEntity* entity, int hitbox)
{
	auto NormalizeNum = [](Vector &vIn, Vector &vOut) -> void
	{
		float flLen = vIn.Length();
		if (flLen == 0) {
			vOut.Init(0, 0, 1);
			return;
		}
		flLen = 1 / flLen;
		vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
	};
	CONST FLOAT MaxDegrees = 180.0f;
	Vector Angles = View;
	Vector Origin = ViewOffSet;
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	Math::AngleVectors(Angles, &Forward);
	Vector AimPos = g_Aimbot->GetHitboxPos(entity, hitbox);
	VectorSubtract(AimPos, Origin, Delta);
	NormalizeNum(Delta, Delta);
	FLOAT DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}

int closest2crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;

	if (!Global::LocalPlayer)
		return -1;

	Vector local_position = Global::LocalPlayer->GetOrigin() + Global::LocalPlayer->ViewOffset();

	Vector angles;
	g_pEngine->GetViewAngles(angles);

	for (int i = 1; i <= g_pGlobals->maxClients; i++)
	{
		CBaseEntity *entity = g_pEntitylist->GetClientEntity(i);

		if (!entity || !entity->IsAlive() || entity->GetTeamNum() == Global::LocalPlayer->GetTeamNum() || entity->IsDormant() || entity == Global::LocalPlayer)
			continue;

		float fov = fov2player(local_position, angles, entity, 0);

		if (fov < lowest_fov)
		{
			lowest_fov = fov;
			index = i;
		}
	}

	return index;
}

float checkWallThickness(CBaseEntity* pPlayer, Vector newhead)
{
	auto leyepos = Global::LocalPlayer->GetOrigin() + Global::LocalPlayer->ViewOffset();

	Vector endpos1, endpos2;
	Vector eyepos = pPlayer->GetOrigin() + pPlayer->ViewOffset();

	Ray_t ray;
	ray.Init(newhead, eyepos);

	CTraceFilterSkipTwoEntities filter(pPlayer, Global::LocalPlayer);

	trace_t trace1, trace2;
	g_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

	if (trace1.DidHit())
		endpos1 = trace1.endpos;
	else
		return 0.f;

	ray.Init(eyepos, newhead);
	g_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

	if (trace2.DidHit())
		endpos2 = trace2.endpos;

	float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
	return endpos1.DistTo(endpos2) + add / 3;
}
#pragma endregion


void CAntiaim::Run(CUserCmd *usercmd)
{
	this->usercmd = usercmd;

	if (!g_Settings->aa_Enabled)
		return;

	if (!Global::LocalPlayer)
		return;

	static int iChokedPackets = -1;

	if ((g_Aimbot->fired && iChokedPackets < 4 && GameUtils::IsAbleToShoot()) && !Global::ForceRealAA)
	{
		Global::SendPacket = false;
		iChokedPackets++;
	}
	else
	{
		iChokedPackets = 0;

		if (!Global::LocalPlayer->IsAlive())
			return;
		if (Global::LocalPlayer->IsProtected())
			return;
		if (*Global::LocalPlayer->GetFlags() & FL_FROZEN)
			return;
		auto animstate = Global::LocalPlayer->GetAnimState();
		if (!animstate)
			return;

		CBaseCombatWeapon* pWeapon = Global::LocalPlayer->GetWeapon();

		if (!pWeapon)
			return;

		if (usercmd->buttons & IN_USE)
			return;

		if (pWeapon)
		{
			CGrenade* csGrenade = (CGrenade*)pWeapon;

			if (pWeapon->IsGrenade())
			{
				if (csGrenade->GetThrowTime() > 0.f)
					return;
			}
			else
			{
				if (pWeapon->IsMiscWeapon() && g_Settings->aa_DisableKnife)
					return;
			}
		}

		choke = !choke;
		Global::SendPacket = choke;

		Fakewalk(usercmd);

		QAngle fucked_angle = usercmd->viewangles;
		static float yaw;
		bool lbybreaking = false;
		float addrandom = Math::RandomFloat(-25.5f, 25.5f);

		if (g_Settings->aa_BreakLby)
		{
			if (g_Settings->aa_Breakertype == 0)
			{
				lbybreaking = LbyBreaker1(usercmd);
			}
			else if (g_Settings->aa_Breakertype == 1)
			{
				lbybreaking = LbyBreaker2(usercmd);
			}
			else if (g_Settings->aa_Breakertype == 2)
			{
				lbybreaking = LbyBreaker3(usercmd);
			}
		}

		if (Global::SendPacket)
		{
			if (g_Settings->aa_FYaw == 1) // static
			{
				if (g_Settings->aa_FYawBase == 3)
					yaw = lbybreaking ? RealYawAngles() + 180.f : RealYawAngles();
				else
					yaw = lbybreaking ? FakeYawAngles() : -FakeYawAngles();
			}
			else if (g_Settings->aa_FYaw == 2) // jitter
			{
				if (g_Settings->aa_FYawBase == 3)
					yaw = lbybreaking ? (RealYawAngles() + 180.f) + addrandom : RealYawAngles() + addrandom;
				else
					yaw = lbybreaking ? FakeYawAngles() + addrandom : -FakeYawAngles() + addrandom;
			}
			else if (g_Settings->aa_FYaw == 3) // spin
			{
				static int Ticks;
				yaw = Ticks;
				Ticks += FakeYawAngles() / 10.f;
				if (Ticks > 240)
					Ticks = 125;
			}

			if (g_Settings->aa_FYawBase == 1) // target
			{
				float best_dist = Global::MainWeapon->GetCSWpnData()->flRange;
				for (int i = 1; i <= g_pGlobals->maxClients; ++i)
				{
					if (i == g_pEngine->GetLocalPlayer())
						continue;
					CBaseEntity* pTarget = g_pEntitylist->GetClientEntity(i);
					Vector pos;
					if (!pTarget || pTarget->GetHealth() < 1)
						continue;
					if (Global::LocalPlayer->GetTeamNum() != pTarget->GetTeamNum())
					{
						pos = pTarget->GetEyePosition();
						float dist = get3ddist2(pos, Global::LocalPlayer->GetEyePosition());
						if (dist >= best_dist)
							continue;
						best_dist = dist;
						QAngle angle = GameUtils::CalculateAngle(Global::LocalPlayer->GetEyePosition(), pos);
						angle.y = Math::NormalizeYaw(angle.y);
						Math::ClampAngles(angle);
						yaw = angle.y;
					}
				}
			}
			else if (g_Settings->aa_FYawBase == 2) // reverse
			{
				if (GetKeyState(g_Settings->aa_Reversekey))
				{
					yaw += 180.f;
				}
			}
		}
		else
		{
			if (animstate->speed_2d > 0.1)
			{
				if (g_Settings->aa_RYawMove == 1) // static
				{
					yaw = -RealYawAngles();
				}
				else if (g_Settings->aa_RYawMove == 2) // jitter
				{
					yaw = -RealYawAngles() + addrandom;
				}
				else if (g_Settings->aa_RYawMove == 3) // spin
				{
					static int Ticks;
					yaw = Ticks;
					Ticks += RealYawAngles() / 10.f;
					if (Ticks > 240)
						Ticks = 125;
				}
			}
			else
			{
				if (g_Settings->aa_RYawStand == 1) // static
				{
					yaw = lbybreaking ? RealYawAngles() : -RealYawAngles();
				}
				else if (g_Settings->aa_RYawStand == 2) // jitter
				{
					yaw = lbybreaking ? RealYawAngles() + addrandom : -RealYawAngles() + addrandom;
				}
				else if (g_Settings->aa_RYawStand == 3) // spin
				{
					static int Ticks;
					yaw = Ticks;
					Ticks += RealYawAngles() / 10.f;
					if (Ticks > 240)
						Ticks = 125;
				}
			}

			if (g_Settings->aa_YawBase == 1) // target
			{
				float best_dist = Global::MainWeapon->GetCSWpnData()->flRange;
				for (int i = 1; i <= g_pGlobals->maxClients; ++i)
				{
					if (i == g_pEngine->GetLocalPlayer())
						continue;

					CBaseEntity* pTarget = g_pEntitylist->GetClientEntity(i);

					Vector pos;
					if (!pTarget || pTarget->GetHealth() < 1)
						continue;

					if (Global::LocalPlayer->GetTeamNum() != pTarget->GetTeamNum())
					{
						pos = pTarget->GetEyePosition();
						float dist = get3ddist2(pos, Global::LocalPlayer->GetEyePosition());

						if (dist >= best_dist)
							continue;

						best_dist = dist;

						QAngle angle = GameUtils::CalculateAngle(Global::LocalPlayer->GetEyePosition(), pos);
						angle.y = Math::NormalizeYaw(angle.y);
						Math::ClampAngles(angle);
						yaw = angle.y;
					}
				}
			}
			else if (g_Settings->aa_YawBase == 2) // reverse
			{
				if (GetKeyState(g_Settings->aa_Reversekey))
				{
					yaw += 180.f;
				}
			}
			else if (g_Settings->aa_YawBase == 3) // freestand
			{
				bool no_active = true;
				float bestrotation = 0.f;
				float highestthickness = 0.f;
				Vector besthead;
				auto leyepos = Global::LocalPlayer->GetOrigin() + Global::LocalPlayer->ViewOffset();
				auto headpos = g_Aimbot->GetHitboxPos(Global::LocalPlayer, 0);
				auto origin = Global::LocalPlayer->GetAbsOrigin();
				int index = closest2crosshair();
				CBaseEntity* entity = nullptr;
				if (index != -1)
					entity = g_pEntitylist->GetClientEntity(index);
				float step = (2 * M_PI) / 8.f;
				float radius = fabs(Vector(headpos - origin).Length2D());
				if (index == -1)
				{
					no_active = true;
				}
				else
				{
					for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
					{
						Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);
						float totalthickness = 0.f;
						no_active = false;
						totalthickness += checkWallThickness(entity, newhead);
						if (totalthickness > highestthickness)
						{
							highestthickness = totalthickness;
							bestrotation = rotation;
							besthead = newhead;
						}
					}
				}
				if (no_active)
					yaw += 180.f;
				else
				{
					yaw = RAD2DEG(bestrotation);
				}
			}
		}

		if (yaw)
		{
			fucked_angle.y += yaw;
		}

#pragma region pitch aa
		if (g_Settings->aa_Pitch == 1) // default
		{
			fucked_angle.x = 89.f;
		}
		else if (g_Settings->aa_Pitch == 2) // minimal
		{
			fucked_angle.x = Global::MainWeapon->IsSniper() ? 82.99f : 89.99f;
		}
		else if (g_Settings->aa_Pitch == 3) // up
		{
			if (g_Settings->misc_AntiUntrust)
			{
				fucked_angle.x = -89.f;
			}
			else
			{
				fucked_angle.x = -991.f;
			}
		}
		else if (g_Settings->aa_Pitch == 4) // down
		{
			if (g_Settings->misc_AntiUntrust)
			{
				fucked_angle.x = 89.f;
			}
			else
			{
				fucked_angle.x = 179.f;
			}
		}
#pragma endregion

		if (Math::ClampAll(fucked_angle))
		{
			usercmd->viewangles = fucked_angle;
		}
	}
}

#pragma region Breakers

float Curtime(CUserCmd* ucmd)
{
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted)
	{
		g_tick = Global::LocalPlayer->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * g_pGlobals->interval_per_tick;
	return curtime;
}

bool CAntiaim::LbyBreaker1(CUserCmd* pcmd)
{
	auto plocal = Global::LocalPlayer;

	if (!plocal)
		return false;

	static float next_update_time;
	auto current_time = Curtime(pcmd);
	bool moving = false;

	auto animstate = plocal->GetAnimState();
	if (!animstate)
		return false;

	if (next_update_time - current_time > 1.1)
		next_update_time = 0;

	if (animstate->speed_2d > 0.1)
		moving = true;

	if (animstate->speed_2d == 0 && !moving)
	{
		if (current_time > next_update_time + 1.1)
		{
			next_update_time = current_time + 1.1;
			return true;
		}
	}
	else if (animstate->speed_2d == 0 && moving)
	{
		if (current_time > next_update_time + 0.22)
		{
			next_update_time = current_time + 1.1;
			moving = false;
			return true;
		}
	}

	return false;
}

bool CAntiaim::LbyBreaker2(CUserCmd* pcmd)
{
	auto plocal = Global::LocalPlayer;

	if (!plocal)
		return false;

	static float next_lby_update_time = 0;
	float curtime = Curtime(pcmd);

	auto animstate = plocal->GetAnimState();
	if (!animstate)
		return false;

	if (!(*plocal->GetFlags() & FL_ONGROUND))
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 + TICKS_TO_TIME(1);


	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}

bool CAntiaim::LbyBreaker3(CUserCmd* pcmd)
{
	auto plocal = Global::LocalPlayer;

	if (!plocal)
		return false;

	static float next_lby_update_time = 0;
	float curtime = Curtime(pcmd);

	auto animstate = plocal->GetAnimState();
	if (!animstate)
		return false;

	if (!(*plocal->GetFlags() & FL_ONGROUND))
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 - TICKS_TO_TIME(1);

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Yaw angles
float CAntiaim::RealYawAngles()
{
	if (Global::LocalPlayer->GetVelocity().Length2D() > 0.1)
		return g_Settings->aa_RYawAngMove;
	else
		return g_Settings->aa_RYawAngStand;
}

float CAntiaim::FakeYawAngles()
{
	return g_Settings->aa_FYawAng;
}
#pragma endregion

#pragma region Fakewalk
void Accelerate(CBaseEntity *player, Vector &wishdir, float wishspeed, float accel, Vector &outVel)
{
	// See if we are changing direction a bit
	float currentspeed = outVel.Dot(wishdir);

	// Reduce wishspeed by the amount of veer.
	float addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	float accelspeed = accel * g_pGlobals->frametime * wishspeed * player->GetSurfaceFriction();

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust velocity.
	for (int i = 0; i < 3; i++)
		outVel[i] += accelspeed * wishdir[i];
}

void WalkMove(CBaseEntity *player, Vector &outVel)
{
	Vector forward, right, up, wishvel, wishdir, dest;
	float_t fmove, smove, wishspeed;

	Math::AngleVectors(player->GetEyeAngles(), forward, right, up);  // Determine movement angles
																	 // Copy movement amounts
	g_pMoveHelper->SetHost(player);
	fmove = g_pMoveHelper->m_flForwardMove;
	smove = g_pMoveHelper->m_flSideMove;
	g_pMoveHelper->SetHost(nullptr);

	if (forward[2] != 0)
	{
		forward[2] = 0;
		Math::NormalizeVector(forward);
	}

	if (right[2] != 0)
	{
		right[2] = 0;
		Math::NormalizeVector(right);
	}

	for (int i = 0; i < 2; i++)	// Determine x and y parts of velocity
		wishvel[i] = forward[i] * fmove + right[i] * smove;

	wishvel[2] = 0;	// Zero out z part of velocity

	wishdir = wishvel; // Determine maginitude of speed of move
	wishspeed = wishdir.Normalize();

	// Clamp to server defined max speed
	g_pMoveHelper->SetHost(player);
	if ((wishspeed != 0.0f) && (wishspeed > g_pMoveHelper->m_flMaxSpeed))
	{
		VectorMultiply(wishvel, player->GetMaxSpeed() / wishspeed, wishvel);
		wishspeed = player->GetMaxSpeed();
	}
	g_pMoveHelper->SetHost(nullptr);
	// Set pmove velocity
	outVel[2] = 0;
	Accelerate(player, wishdir, wishspeed, g_pCvar->FindVar("sv_accelerate")->GetFloat(), outVel);
	outVel[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(outVel, player->GetBaseVelocity(), outVel);

	float spd = outVel.Length();

	if (spd < 1.0f)
	{
		outVel.Init();
		// Now pull the base velocity back out. Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
		return;
	}

	g_pMoveHelper->SetHost(player);
	g_pMoveHelper->m_outWishVel += wishdir * wishspeed;
	g_pMoveHelper->SetHost(nullptr);

	// Don't walk up stairs if not on ground.
	if (!(*player->GetFlags() & FL_ONGROUND))
	{
		// Now pull the base velocity back out. Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
		return;
	}

	// Now pull the base velocity back out. Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
}

void CAntiaim::Fakewalk(CUserCmd *userCMD)
{
	if (!g_Settings->misc_FakewalkEnabled || !GetAsyncKeyState(g_Settings->misc_FakewalkKey))
		return;

	if (Global::bShouldChoke)
		Global::SendPacket = Global::bShouldChoke = false;

	if (!Global::SendPacket)
		Global::nChockedTicks++;
	else
		Global::nChockedTicks = 0;

	Vector velocity = Global::vecUnpredictedVel;

	int Iterations = 0;
	for (; Iterations < 15; ++Iterations)
	{
		if (velocity.Length() < 0.1)
		{
			//Msg("[Fakewalk]: Ticks till stop %d\n", Iterations);
			break;
		}

		Friction(velocity);
		WalkMove(Global::LocalPlayer, velocity);

	}

	int choked_ticks = Global::nChockedTicks;

	if (Iterations > 7 - choked_ticks || !choked_ticks)
	{
		float_t speed = velocity.Length();

		QAngle direction;
		Math::VectorAngles(velocity, direction);

		direction.y = userCMD->viewangles.y - direction.y;

		Vector forward;
		Math::AngleVectors(direction, &forward);
		Vector negated_direction = forward * -speed;

		userCMD->forwardmove = negated_direction.x;
		userCMD->sidemove = negated_direction.y;
	}

	if (Global::nChockedTicks < 7)
		Global::bShouldChoke = true;

}

void CAntiaim::Friction(Vector &outVel)
{
	float speed, newspeed, control;
	float friction;
	float drop;

	speed = outVel.Length();

	if (speed <= 0.1f)
		return;

	drop = 0;

	// apply ground friction
	if (*Global::LocalPlayer->GetFlags() & FL_ONGROUND)
	{
		friction = g_pCvar->FindVar("sv_friction")->GetFloat() * Global::LocalPlayer->GetSurfaceFriction();

		// Bleed off some speed, but if we have less than the bleed
		// threshold, bleed the threshold amount.
		control = (speed < g_pCvar->FindVar("sv_stopspeed")->GetFloat()) ? g_pCvar->FindVar("sv_stopspeed")->GetFloat() : speed;

		// Add the amount to the drop amount.
		drop += control * friction * g_pGlobals->frametime;
	}

	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	if (newspeed != speed)
	{
		// Determine proportion of old speed we are using.
		newspeed /= speed;
		// Adjust velocity according to proportion.
		VectorMultiply(outVel, newspeed, outVel);
	}
}
#pragma endregion


void freestand(CUserCmd* cmd)
{
	if (!Global::LocalPlayer)
		return;

	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	auto leyepos = Global::LocalPlayer->GetOrigin() + Global::LocalPlayer->ViewOffset();
	auto headpos = g_Aimbot->GetHitboxPos(Global::LocalPlayer, 0);
	auto origin = Global::LocalPlayer->GetAbsOrigin();

	int index = closest2crosshair();

	CBaseEntity* entity = nullptr;

	if (index != -1)
		entity = g_pEntitylist->GetClientEntity(index);

	float step = (2 * M_PI) / 8.f;

	float radius = fabs(Vector(headpos - origin).Length2D());

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

			float totalthickness = 0.f;

			no_active = false;

			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}

	/*if (GLOBAL::should_send_packet)
		cmd->viewangles.y += randnum(-180, 180);
	else
	{
		if (next_lby_update(cmd->viewangles.y + Vars::options.delta_val, cmd))
		{
			cmd->viewangles.y = last_real + Vars::options.delta_val;
		}
		else
		{
			if (no_active)
				cmd->viewangles.y += 180.f;
			else
				cmd->viewangles.y = RAD2DEG(bestrotation);

			last_real = cmd->viewangles.y;
		}
	}*/
}