#include "Aimbot.h"
#include "Math.h"
#include "global.h"
#include "GameUtils.h"
#include "Autowall.h"

//	-Vars-	//
int bestHitbox = -1, mostDamage;
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

#pragma region Backtrack stuff

static ConVar *big_ud_rate = nullptr;
static ConVar *min_ud_rate = nullptr;
static ConVar *max_ud_rate = nullptr;
static ConVar *interp_ratio = nullptr;
static ConVar *cl_interp = nullptr;
static ConVar *cl_min_interp = nullptr;
static ConVar *cl_max_interp = nullptr;

float LerpTime()
{
	static ConVar* updaterate = g_pCvar->FindVar("cl_updaterate");
	static ConVar* minupdate = g_pCvar->FindVar("sv_minupdaterate");
	static ConVar* maxupdate = g_pCvar->FindVar("sv_maxupdaterate");
	static ConVar* lerp = g_pCvar->FindVar("cl_interp");
	static ConVar* cmin = g_pCvar->FindVar("sv_client_min_interp_ratio");
	static ConVar* cmax = g_pCvar->FindVar("sv_client_max_interp_ratio");
	static ConVar* ratio = g_pCvar->FindVar("cl_interp_ratio");

	float lerpurmom = lerp->GetFloat();
	float maxupdateurmom = maxupdate->GetFloat();
	int updaterateurmom = updaterate->GetInt();
	float ratiourmom = ratio->GetFloat();
	int sv_maxupdaterate = maxupdate->GetInt();
	int sv_minupdaterate = minupdate->GetInt();
	float cminurmom = cmin->GetFloat();
	float cmaxurmom = cmax->GetFloat();

	if (sv_maxupdaterate && sv_minupdaterate)
		updaterateurmom = maxupdateurmom;

	if (ratiourmom == 0)
		ratiourmom = 1.0f;

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratiourmom = clamp(ratiourmom, cminurmom, cmaxurmom);

	return max(lerpurmom, ratiourmom / updaterateurmom);
}

bool CAimbot::GoodBacktrackTick(int tick)
{
	auto nci = g_pEngine->GetNetChannelInfo();

	if (!nci) {
		return false;
	}

	float correct = clamp(nci->GetLatency(FLOW_OUTGOING) + LerpTime(), 0.f, 1.f);

	float delta_time = correct - (g_pGlobals->curtime - TICKS_TO_TIME(tick));

	return fabsf(delta_time) < 0.2f;
}

void CAimbot::LbyBacktrack(CUserCmd *pCmd, CBaseEntity* pLocal, CBaseEntity* pEntity)
{
	int index = pEntity->GetIndex();
	float PlayerVel = abs(pEntity->GetVelocity().Length2D());
	auto& pset = g_PlayerSettings[index];
	bool playermoving;

	if (PlayerVel > 0.f)
		playermoving = true;
	else
		playermoving = false;

	float lby = pEntity->LowerBodyYaw();
	static float lby_timer[65];
	static float lby_proxy[65];
	float server_time = pLocal->GetTickBase() * g_pGlobals->interval_per_tick;

	if (lby_proxy[index] != pEntity->LowerBodyYaw() && playermoving == false)
	{
		lby_timer[index] = 0;
		lby_proxy[index] = pEntity->LowerBodyYaw();
	}

	if (playermoving == false)
	{
		if (server_time >= lby_timer[index])
		{
			pset.tickToBack = pEntity->GetSimulationTime();
			pset.lbyBacktrack = pEntity->LowerBodyYaw();
			lby_timer[index] = server_time + 1.1;
		}
	}
	else
	{
		pset.tickToBack = 0;
		lby_timer[index] = 0;
	}

	if (GoodBacktrackTick(TIME_TO_TICKS(pset.tickToBack)))
		pset.backtrackTick = true;
	else
		pset.backtrackTick = false;
}
#pragma endregion


float CAimbot::HitChance(CBaseCombatWeapon* weapon)
{
	auto local_player = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (!local_player)
		return 0;

	if (!weapon)
		return 0;

	float hitchance = 101;
	float inaccuracy = weapon->GetInaccuracy();

	if (inaccuracy == 0)
		inaccuracy = 0.0000001;

	inaccuracy = 1 / inaccuracy;
	hitchance = inaccuracy;

	return hitchance;
}

void CAimbot::Run(CUserCmd* cmd)
{
	for (int i = 1; i < 65; i++)
	{
		auto entity = g_pEntitylist->GetClientEntity(i);

		if (!entity)
			continue;

		if (!Global::LocalPlayer)
			continue;

		bool is_local_player = entity == Global::LocalPlayer;
		bool is_teammate = Global::LocalPlayer->GetTeamNum() == entity->GetTeamNum() && !is_local_player;
		auto& pset = g_PlayerSettings[entity->GetIndex()];

		if (is_local_player)
			continue;

		if (is_teammate)
			continue;

		if (entity->GetHealth() <= 0)
			continue;

		if (Global::LocalPlayer->GetHealth() <= 0)
			continue;

		if (!entity->IsAlive())
			continue;

		if (!Global::LocalPlayer->IsAlive())
			continue;

		if (entity->IsProtected())
			continue;

		auto weapon = reinterpret_cast<CBaseCombatWeapon*>(g_pEntitylist->GetClientEntity(Global::LocalPlayer->GetActiveWeaponIndex()));

		if (!weapon)
			return;

		if (weapon->GetCSWpnData()->WeaponType == 9 || weapon->GetCSWpnData()->WeaponType == 0)
			return;

		if (weapon->GetLoadedAmmo() == 0)
			return;

		QAngle view11; g_pEngine->GetViewAngles(view11);

		Vector local_position = Global::LocalPlayer->GetOrigin() + Global::LocalPlayer->ViewOffset();
		Vector scan_point;

		if (g_Settings->rage_Enabled && g_Settings->rage_AutoRevolver)
		{
			if (weapon->WeaponID() == ItemDefinitionIndex::REVOLVER) {
				{
					cmd->buttons |= IN_ATTACK;
					float flPostponeFireReady = weapon->GetPostponeFireReadyTime();
					if (flPostponeFireReady > 0 && flPostponeFireReady < g_pGlobals->curtime) {
						cmd->buttons &= ~IN_ATTACK;
					}
				}
			}
		}

		if (g_Settings->rage_Hitscan == 0)
		{
			scan_point = GetHitboxPos(entity, 0);
		}
		else if (g_Settings->rage_Hitscan == 1)
		{
			if (g_Settings->rage_MultipointEnable)
				scan_point = ScanPoint(entity);
			else
				scan_point = GetHitboxPos(entity, ScanHitbox(entity));
		}

		LbyBacktrack(cmd, Global::LocalPlayer, entity);

		float fov = GameUtils::GetFov(view11, Global::LocalPlayer->GetEyePosition(), entity->GetEyePosition(), false);

		if (fov > 180.f)
			continue;

		if (scan_point == Vector(0, 0, 0))
			continue;

		float selection = 0;
		switch (g_Settings->rage_Selection)
		{
		case 0:
			selection = fov;
			break;
		case 1:
			selection = entity->GetVelocity().Length();
			break;
		case 2:
			selection = entity->GetHealth();
			break;

		default:
			break;
		}
		if (bestdist >= selection)
			bestdist = selection;

		if (g_Settings->rage_Enabled)
		{
			if (g_Autowall->CanHit(local_position, scan_point) > g_Settings->rage_Mindamage && CanShoot())
			{
				cmd->viewangles = Math::NormalizeAngle(Math::CalcAngle(local_position, scan_point));

				if (g_Settings->rage_Onkey && GetKeyState(g_Settings->rage_Key))
					return;

				if (g_Settings->rage_AutoShot)
				{
					if (g_Settings->rage_AutoScope && weapon->IsScopeable() && !Global::LocalPlayer->IsScoped())
						cmd->buttons |= IN_ATTACK2;
					else
					{
						if (HitChance(weapon) > g_Settings->rage_Hitchance)
						{
							cmd->buttons |= IN_ATTACK;
							this->fired = true;
						}
					}
				}

				if (CanShoot())
					pset.shotsFired++;

				if (entity->GetVelocity().Length2D() > 0)
				{
					if (CanShoot() && GoodBacktrackTick(TIME_TO_TICKS(entity->GetSimulationTime())))
						cmd->tick_count = TIME_TO_TICKS(entity->GetSimulationTime() + LerpTime());
				}
				else
				{
					if (pset.backtrackTick)
						cmd->tick_count = TIME_TO_TICKS(pset.tickToBack + LerpTime());
				}
			}
		}
	}
}

#pragma region Hitscan
int CAimbot::ScanHitbox(CBaseEntity* entity)
{
	auto local_player = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetOrigin() + local_player->ViewOffset();
	static int hitboxes[] =
	{
		HITBOX_HEAD,
		HITBOX_NECK,
		HITBOX_UPPER_CHEST,
		HITBOX_THORAX,
		HITBOX_LOWER_CHEST,
		HITBOX_BELLY,
		HITBOX_PELVIS,
		HITBOX_LEFT_UPPER_ARM,
		HITBOX_LEFT_FOREARM,
		HITBOX_RIGHT_UPPER_ARM,
		HITBOX_RIGHT_FOREARM,
		HITBOX_LEFT_THIGH,
		HITBOX_LEFT_CALF,
		HITBOX_LEFT_FOOT,
		HITBOX_RIGHT_THIGH,
		HITBOX_RIGHT_CALF,
		HITBOX_RIGHT_FOOT
	};
	mostDamage = g_Settings->rage_Mindamage;

	for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
	{
		if (!g_Settings->rage_Hitboxes[i]) // perfect custom system lol
			continue;

		Vector point = GetHitboxPos(entity, hitboxes[i]);
		int damage = g_Autowall->CanHit(local_position, point);
		if (damage > mostDamage)
		{
			bestHitbox = hitboxes[i];
			mostDamage = damage;
		}
		if (damage >= entity->GetHealth())
		{
			bestHitbox = hitboxes[i];
			mostDamage = damage;
			break;
		}
	}
	return bestHitbox;
}

Vector CAimbot::ScanPoint(CBaseEntity* entity)
{
	auto local_player = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (!local_player)
		return GetHitboxPos(entity, 0);

	Vector local_position = local_player->GetOrigin() + local_player->ViewOffset();
	Vector vector_best_point;
	static int hitboxes[] =
	{
		HITBOX_HEAD,
		HITBOX_NECK,
		HITBOX_UPPER_CHEST,
		HITBOX_THORAX,
		HITBOX_LOWER_CHEST,
		HITBOX_BELLY,
		HITBOX_PELVIS,
		HITBOX_LEFT_UPPER_ARM,
		HITBOX_LEFT_FOREARM,
		HITBOX_RIGHT_UPPER_ARM,
		HITBOX_RIGHT_FOREARM,
		HITBOX_LEFT_THIGH,
		HITBOX_LEFT_CALF,
		HITBOX_LEFT_FOOT,
		HITBOX_RIGHT_THIGH,
		HITBOX_RIGHT_CALF,
		HITBOX_RIGHT_FOOT
	};
	QAngle view11; g_pEngine->GetViewAngles(view11);
	mostDamage = g_Settings->rage_Mindamage;

	matrix3x4_t matrix[128];
	if (!entity->SetupBones2(matrix, 128, 256, 0))
		return GetHitboxPos(entity, 0);

	for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
	{
		float fov = GameUtils::GetFov(view11, Global::LocalPlayer->GetEyePosition(), entity->GetEyePosition(), true);
		// true -> distance based; false -> none
		if (fov > g_Settings->rage_MultipDistance)
			continue;

		if (bestdist >= fov)
			bestdist = fov; // perfect fov selection system lol

		if (!g_Settings->rage_Hitboxes[i]) // perfect custom system lol
			continue;

		for (auto point : GameUtils::HitboxMultipoint(entity, hitboxes[i], matrix))
		{
			int damage = g_Autowall->CanHit(local_position, point);

			if (!damage) continue;

			if (damage > mostDamage)
			{
				mostDamage = damage;
				vector_best_point = point;
				bestHitbox = hitboxes[i];
			}
			if (damage >= entity->GetHealth())
			{
				mostDamage = damage;
				vector_best_point = point;
				bestHitbox = hitboxes[i];
			}
		}
	}
	return vector_best_point;
}

mstudiobbox_t* CAimbot::GetHitbox(CBaseEntity* entity, int hitbox_index)
{
	if (entity->IsDormant() || entity->GetHealth() <= 0 || !entity->IsAlive())
		return NULL;

	const auto pModel = entity->GetModel();
	if (!pModel)
		return NULL;

	auto pStudioHdr = g_pModelInfo->GetStudioModel(pModel);
	if (!pStudioHdr)
		return NULL;

	auto pSet = pStudioHdr->pHitboxSet(0);
	if (!pSet)
		return NULL;

	if (hitbox_index >= pSet->numhitboxes || hitbox_index < 0)
		return NULL;

	return pSet->pHitbox(hitbox_index);
}

Vector CAimbot::GetHitboxPos(CBaseEntity* entity, int hitbox_id)
{
	auto hitbox = GetHitbox(entity, hitbox_id);
	if (!hitbox)
		return Vector(0, 0, 0);

	auto bone_matrix = entity->GetBoneMatrix(hitbox->bone);

	Vector bbmin, bbmax;
	GameUtils::VectorTransform(hitbox->bbmin, bone_matrix, bbmin);
	GameUtils::VectorTransform(hitbox->bbmax, bone_matrix, bbmax);

	return (bbmin + bbmax) * 0.5f;
}
#pragma endregion

float GetCurtime()
{
	if (!Global::LocalPlayer)
		return -1;

	return static_cast<float>(Global::LocalPlayer->GetTickBase()) * g_pGlobals->interval_per_tick;
}

bool CAimbot::CanShoot()
{
	if (!Global::LocalPlayer || Global::LocalPlayer->GetHealth() <= 0 || !Global::LocalPlayer->IsAlive())
		return false;

	auto weapon = reinterpret_cast<CBaseCombatWeapon*>(g_pEntitylist->GetClientEntity(Global::LocalPlayer->GetActiveWeaponIndex()));

	if (!weapon)
		return false;

	return (weapon->NextPrimaryAttack() < GetCurtime()) && (Global::LocalPlayer->GetNextAttack() < GetCurtime());
}

void CAimbot::RemoveRecoil(CUserCmd* cmd)
{
	if (cmd->buttons & IN_ATTACK)
	{
		if (g_Settings->rage_Norecoil)
		{
			ConVar* recoilscale = g_pCvar->FindVar("weapon_recoil_scale");

			if (recoilscale)
			{
				QAngle qPunchAngles = Global::LocalPlayer->GetPunchAngle();
				QAngle qAimAngles = cmd->viewangles;
				qAimAngles -= qPunchAngles * recoilscale->GetFloat();
				cmd->viewangles = qAimAngles;
			}
		}
	}
}

CAimbot* g_Aimbot = new CAimbot();