#include "hooks.h"
#include <time.h>
#include "Mmsystem.h"
#include <thread>
#include "global.h"
#include "Hitmarker.h"
#include "Resolver.h"

#pragma comment(lib, "winmm.lib") 

#define EVENT_HOOK( x )

cGameEvent g_Event;
std::vector<trace_info> trace_logs;

char* HitgroupToName(int hitgroup)
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return "leg";
	case HITGROUP_STOMACH:
		return "stomach";
	default:
		return "body";
	}
}

void cGameEvent::FireGameEvent(IGameEvent *event)
{
	const char* szEventName = event->GetName();
	if (!szEventName)
		return;

	auto& resolver = CResolver::Get();

	if (strcmp(szEventName, "round_start") == 0)
	{
		for (int i = 1; i <= g_pGlobals->maxClients; i++)
		{
			auto entity = g_pEntitylist->GetClientEntity(i);
			if (!entity)
				continue;
			if (!Global::LocalPlayer)
				continue;
			bool is_local_player = entity == Global::LocalPlayer;
			bool is_teammate = Global::LocalPlayer->GetTeamNum() == entity->GetTeamNum() && !is_local_player;
			auto& pset = g_PlayerSettings[i];
			auto& resinfo = resolver.res_info[i];
			if (is_local_player)
				continue;
			if (is_teammate)
				continue;
			if (Global::LocalPlayer->GetHealth() <= 0)
				continue;
			if (!Global::LocalPlayer->IsAlive())
				continue;

			if (g_Settings->rage_ResolverClearRecs)
			{
				pset.shotsHit = 0;
				pset.shotsMissed = 0;
				pset.shotsFired = 0;
				resinfo.ms_air = 0;
				resinfo.ms_blby = 0;
				resinfo.ms_delta120 = 0;
				resinfo.ms_delta35 = 0;
				resinfo.ms_fhead = 0;
				resinfo.ms_fwalk = 0;
			}
		}
	}

	if (g_Settings->visuals_DamageIndicator)
	{
		if (strcmp(szEventName, "player_hurt") == 0)
		{
			auto attacker = event->GetInt("attacker");
			auto dmgdone = event->GetInt("dmg_health");
			auto Hurt = event->GetInt("userid");
			CBaseEntity* pEnt = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(Hurt));
			if (g_pEngine->GetPlayerForUserID(attacker) == g_pEngine->GetLocalPlayer()) {
				Global::DamageDealt = dmgdone;
				Global::DamageHit.push_back(FloatingText(pEnt, 1.f, dmgdone));

			}
		}
	}

	if (strcmp(szEventName, "player_hurt") == 0)
	{
		int CBaseEntityServerID = event->GetInt(XorStr("userID"));
		int AttackerServerID = event->GetInt(XorStr("attacker"));
		int AttackerIndex = g_pEngine->GetPlayerForUserID(AttackerServerID);
		if (AttackerIndex != g_pEngine->GetLocalPlayer())
			return;
		int CBaseEntityIndex = g_pEngine->GetPlayerForUserID(CBaseEntityServerID);
		if (CBaseEntityIndex == g_pEngine->GetLocalPlayer())
			return;

		CHitmarker::Get().update_end_time();
		CHitmarker::Get().play_sound();

		player_info_s pl_info;
		g_pEngine->GetPlayerInfo(CBaseEntityIndex, &pl_info);

		bool IsHeadshot = (event->GetInt(XorStr("hitgroup")) == 1);
		auto& resinfo = resolver.res_info[CBaseEntityIndex];
		auto& psets = g_PlayerSettings[CBaseEntityIndex];
			
		psets.shotsHit++;

		if (IsHeadshot)
		{
			psets.shotsMissed = 0;
			resinfo.lasthitangle = resinfo.lastsetangle;
		}
	}

	if (g_Settings->misc_Logdmg)
	{
		if (strcmp(szEventName, "player_hurt") == 0)
		{
			int iAttacker = g_pEngine->GetPlayerForUserID(event->GetInt("attacker"));
			int iVictim = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));
			if (iAttacker == g_pEngine->GetLocalPlayer() && iVictim != g_pEngine->GetLocalPlayer())
			{
				player_info_t pinfo;
				g_pEngine->GetPlayerInfo(iVictim, &pinfo);
				g_pCvar->ConsoleColorPrintf(Color(37, 195, 219, 255), "[Oestrum] ");
				Msg("Hit %s in the %s for %d damage (%d health remaining) \n", pinfo.m_szPlayerName, HitgroupToName(event->GetInt("hitgroup")), event->GetInt("dmg_health"), event->GetInt("health"));
			}
		}
	}

	if (strcmp(szEventName, "bullet_impact") == 0)
	{
		auto* index = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

		Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

		if (index)
			trace_logs.push_back(trace_info(index->GetEyePosition(), position, g_pGlobals->curtime, event->GetInt("userid")));
	}
}

int cGameEvent::GetEventDebugID()
{
	return 42;
}

void cGameEvent::RegisterSelf()
{
	g_pGameEventManager->AddListener(this, "player_hurt", false);
	g_pGameEventManager->AddListener(this, "round_start", false);
	g_pGameEventManager->AddListener(this, "round_end", false);
	g_pGameEventManager->AddListener(this, "player_death", false);
	g_pGameEventManager->AddListener(this, "weapon_fire", false);
	g_pGameEventManager->AddListener(this, "bullet_impact", false);
}

void cGameEvent::Register()
{
	EVENT_HOOK(FireEvent);
}