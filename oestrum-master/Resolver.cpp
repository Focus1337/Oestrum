#include "Resolver.h"
#include "global.h"
#include "Math.h"
#include "GameUtils.h"
#include "Aimbot.h"

void CResolver::UpdateLogs()
{
	for (int i = 1; i <= g_pGlobals->maxClients; i++)
	{
		auto &info = res_info[i];

		CBaseEntity *entity = g_pEntitylist->GetClientEntity(i);
		if (!entity || !entity->IsAlive() || entity->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
		{
			info.flag_active = false;
			continue;
		}

		if (entity->IsDormant())
			continue;

		if (info.rec_simtime == entity->GetSimulationTime())
			continue;

		info.SaveRecord(entity);
		info.flag_active = true;
	}
}

void CResolver::ResolvePlayers(CBaseEntity* entity)
{
	if (!entity)
		return;
	if (!Global::LocalPlayer)
		return;
	bool is_local_player = entity == Global::LocalPlayer;
	if (is_local_player)
		return;
	if (entity->GetTeamNum() == Global::LocalPlayer->GetTeamNum() && !is_local_player)
		return;
	if (entity->GetHealth() <= 0)
		return;
	if (Global::LocalPlayer->GetHealth() <= 0)
		return;

	int idx = entity->GetIndex();
	auto& info = res_info[idx];
	auto& pset = g_PlayerSettings[idx];

	if (!info.flag_active)
		return;

	info.res_lby = Vector(entity->GetEyeAnglesPtr()->x, entity->LowerBodyYaw(), 0.f);
	info.res_inverselby = Vector(entity->GetEyeAnglesPtr()->x, entity->LowerBodyYaw() + 180.f, 0.f);
	info.res_lastlby = Vector(entity->GetEyeAnglesPtr()->x, info.rec_lastmovinglby, 0.f);
	info.res_backtracklby = Vector(entity->GetEyeAnglesPtr()->x, pset.lbyBacktrack, 0.f);
	info.res_lbydelta = Vector(entity->GetEyeAnglesPtr()->x, entity->LowerBodyYaw() + info.rec_lbydelta, 0.f);

	info.flag_ismoving = ((entity->GetVelocity().Length2D() > 0.1) && (*entity->GetFlags() & FL_ONGROUND));
	info.flag_isinair = (!(*entity->GetFlags() & FL_ONGROUND));
	info.flag_isducking = (*entity->GetFlags() & FL_DUCKING);

	pset.shotsMissed = pset.shotsFired - pset.shotsHit;

	AnimationLayer curBalanceLayer, prevBalanceLayer;

	ResolveInfo curtickrecord;
	curtickrecord.SaveRecord(entity);

	if (pset.backtrackTick)
	{
		pset.resolveType = "Back lby";
		entity->SetEyeAngles(info.res_backtracklby);
	}
	else if (info.flag_isinair)
	{
		switch (info.ms_air % 5)
		{
		case 0:
		{pset.resolveType = "Air (1)"; entity->SetEyeAngles(info.res_lby); break; }
		case 1:
		{pset.resolveType = "Air (2)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 15.f, 0.f)); break; }
		case 2:
		{pset.resolveType = "Air (3)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 30.f, 0.f)); break; }
		case 3:
		{pset.resolveType = "Air (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 30.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Air (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 45.f, 0.f)); break; }
		}
		info.ms_air = pset.shotsMissed;
	}
	else if (info.flag_ismoving)
	{
		pset.resolveType = "Moving";
		entity->SetEyeAngles(info.res_lby);
		info.rec_lastmovinglby = entity->LowerBodyYaw();
	}
	else if (IsFakewalking(entity, curtickrecord) && (info.flag_ismoving && !info.flag_isducking))
	{
		switch (info.ms_fwalk % 5)
		{
		case 0:
		{pset.resolveType = "Fakewalking (1)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 1:
		{pset.resolveType = "Fakewalking (2)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 2:
		{pset.resolveType = "Fakewalking (3)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 3:
		{pset.resolveType = "Fakewalking (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Fakewalking (5)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		}
		info.ms_fwalk = pset.shotsMissed;
	}
	else if (ABDeltaLess120(entity, curtickrecord, curBalanceLayer, prevBalanceLayer) && !info.flag_ismoving && !info.flag_isinair)
	{
		info.rec_lbydelta = info.rec_oldlby - entity->LowerBodyYaw();
		switch (info.ms_delta120 % 8)
		{
		case 0:
		{pset.resolveType = "Delta 120 (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1:
		{pset.resolveType = "Delta 120 (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2:
		{pset.resolveType = "Delta 120 (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3:
		{pset.resolveType = "Delta 120 (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Delta 120 (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5:
		{pset.resolveType = "Delta 120 (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6:
		{pset.resolveType = "Delta 120 (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7:
		{pset.resolveType = "Delta 120 (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
		info.ms_delta120 = pset.shotsMissed;
	}
	else if (ABDeltaLess35(entity, curtickrecord, curBalanceLayer, prevBalanceLayer) && !info.flag_ismoving && !info.flag_isinair)
	{
		info.rec_lbydelta = info.rec_lastmovinglby - entity->LowerBodyYaw();
		switch (info.ms_delta35 % 8)
		{
		case 0: // sometimes p
		{pset.resolveType = "Delta 35 (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1: // sometimes p
		{pset.resolveType = "Delta 35 (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2: // idk
		{pset.resolveType = "Delta 35 (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3: // idk
		{pset.resolveType = "Delta 35 (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4: // hittin p
		{pset.resolveType = "Delta 35 (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5: // hittin p
		{pset.resolveType = "Delta 35 (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6: // idk
		{pset.resolveType = "Delta 35 (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7: // idk
		{pset.resolveType = "Delta 35 (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
		info.ms_delta35 = pset.shotsMissed;
	}
	else if (ABBreakLby(entity, curtickrecord, curBalanceLayer, prevBalanceLayer) && !info.flag_ismoving && !info.flag_isinair)
	{
		switch (info.ms_blby % 8)
		{
		case 0:
		{pset.resolveType = "Break lby (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1:
		{pset.resolveType = "Break lby (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2:
		{pset.resolveType = "Break lby (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3:
		{pset.resolveType = "Break lby (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Break lby (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5:
		{pset.resolveType = "Break lby (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6:
		{pset.resolveType = "Break lby (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7:
		{pset.resolveType = "Break lby (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
		info.ms_blby = pset.shotsMissed;
	}
	else if (ABFakeHead(entity, curtickrecord, curBalanceLayer) && !info.flag_ismoving && !info.flag_isinair)
	{
		switch (info.ms_fhead % 8)
		{
		case 0:
		{pset.resolveType = "Fake head (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1:
		{pset.resolveType = "Fake head (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2:
		{pset.resolveType = "Fake head (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3:
		{pset.resolveType = "Fake head (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Fake head (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5:
		{pset.resolveType = "Fake head (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6:
		{pset.resolveType = "Fake head (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7:
		{pset.resolveType = "Fake head (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
		info.ms_fhead = pset.shotsMissed;
	}
	else if (entity->GetSimulationTime() >= info.rec_standingtime + 0.22f && !info.flag_ismoving && !info.flag_isinair)
	{
		info.rec_lbydelta = info.rec_oldlby - entity->LowerBodyYaw();
		info.ms_delta120 = pset.shotsMissed;
		switch (info.ms_delta120 % 8)
		{
		case 0:
		{pset.resolveType = "Delta 120 (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1:
		{pset.resolveType = "Delta 120 (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2:
		{pset.resolveType = "Delta 120 (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3:
		{pset.resolveType = "Delta 120 (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Delta 120 (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5:
		{pset.resolveType = "Delta 120 (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6:
		{pset.resolveType = "Delta 120 (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7:
		{pset.resolveType = "Delta 120 (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
	}
	else if (entity->GetSimulationTime() >= info.rec_standingtime + 1.32f && std::fabsf(info.rec_lbydelta) < 35.f && !info.flag_ismoving && !info.flag_isinair)
	{
		info.rec_lbydelta = info.rec_lastmovinglby - entity->LowerBodyYaw();
		info.ms_delta35 = pset.shotsMissed;
		switch (info.ms_delta35 % 8)
		{
		case 0:
		{pset.resolveType = "Delta 35 (1)"; entity->SetEyeAngles(info.res_lbydelta); break; }
		case 1:
		{pset.resolveType = "Delta 35 (2)"; entity->SetEyeAngles(info.res_inverselby); break; }
		case 2:
		{pset.resolveType = "Delta 35 (3)"; entity->SetEyeAngles(info.res_lastlby); break; }
		case 3:
		{pset.resolveType = "Delta 35 (4)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 35.f, 0.f)); break; }
		case 4:
		{pset.resolveType = "Delta 35 (5)"; entity->SetEyeAngles(info.res_lastlby + Vector(0.f, 120.f, 0.f)); break; }
		case 5:
		{pset.resolveType = "Delta 35 (6)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 35.f, 0.f)); break; }
		case 6:
		{pset.resolveType = "Delta 35 (7)"; entity->SetEyeAngles(info.res_lby + Vector(0.f, 120.f, 0.f)); break; }
		case 7:
		{pset.resolveType = "Delta 35 (8)"; entity->SetEyeAngles(info.res_lbydelta + Vector(0.f, 35.f, 0.f)); break; }
		}
	}
	else
	{
		pset.resolveType = "Lby";
		entity->SetEyeAngles(info.res_lby);
	}

	pset.lbyUpdateTime = info.rec_standingtime;
}


bool CResolver::IsAdjustingBalance(CBaseEntity *entity, ResolveInfo &record, AnimationLayer *layer)
{
	for (int i = 0; i < record.rec_layercount; i++)
	{
		const int activity = entity->GetSequenceActivity(record.rec_animlayer[i].m_nSequence);
		if (activity == 979)
		{
			*layer = record.rec_animlayer[i];
			return true;
		}
	}
	return false;
}

bool CResolver::IsAdjustingStopMoving(CBaseEntity *entity, ResolveInfo &record, AnimationLayer *layer)
{
	for (int i = 0; i < record.rec_layercount; i++)
	{
		const int activity = entity->GetSequenceActivity(record.rec_animlayer[i].m_nSequence);
		if (activity == 980)
		{
			*layer = record.rec_animlayer[i];
			return true;
		}
	}
	return false;
}

bool CResolver::IsFakewalking(CBaseEntity* entity, ResolveInfo &record)
{
	bool
		bFakewalking = false,
		stage1 = false,			// stages needed cause we are iterating all layers, eitherwise won't work :)
		stage2 = false,
		stage3 = false;

	for (int i = 0; i < record.rec_layercount; i++)
	{
		if (record.rec_animlayer[i].m_nSequence == 26 && record.rec_animlayer[i].m_flWeight < 0.4f)
			stage1 = true;
		if (record.rec_animlayer[i].m_nSequence == 7 && record.rec_animlayer[i].m_flWeight > 0.001f)
			stage2 = true;
		if (record.rec_animlayer[i].m_nSequence == 2 && record.rec_animlayer[i].m_flWeight == 0)
			stage3 = true;
	}

	if (stage1 && stage2)
		if (stage3 || (*entity->GetFlags() & FL_DUCKING)) // since weight from stage3 can be 0 aswell when crouching, we need this kind of check, cause you can fakewalk while crouching, thats why it's nested under stage1 and stage2
			bFakewalking = true;
		else
			bFakewalking = false;
	else
		bFakewalking = false;

	return bFakewalking;
}

bool CResolver::ABDeltaLess120(CBaseEntity* entity, ResolveInfo &record, AnimationLayer &curlayer, AnimationLayer &prevlayer)
{
	if (IsAdjustingBalance(entity, record, &curlayer))
	{
		if (IsAdjustingBalance(entity, record, &prevlayer))
		{
			if (curlayer.m_flWeight == 0.f && (prevlayer.m_flCycle > 0.92f && curlayer.m_flCycle > 0.92f))
			{
				if (entity->GetSimulationTime() >= res_info[entity->GetIndex()].rec_standingtime + 0.22f /*&& !res_info[entity->GetIndex()].flag_ismoving*/)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool CResolver::ABDeltaLess35(CBaseEntity* entity, ResolveInfo &record, AnimationLayer& curlayer, AnimationLayer& prevlayer)
{
	if (IsAdjustingBalance(entity, record, &curlayer))
	{
		if (IsAdjustingBalance(entity, record, &prevlayer))
		{
			if (curlayer.m_flWeight == 0.f && (prevlayer.m_flCycle > 0.92f && curlayer.m_flCycle > 0.92f))
			{
				if (entity->GetSimulationTime() >= res_info[entity->GetIndex()].rec_standingtime + 1.32f && std::fabsf(res_info[entity->GetIndex()].rec_lbydelta) < 35.f)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool CResolver::ABBreakLby(CBaseEntity* entity, ResolveInfo &record, AnimationLayer& curlayer, AnimationLayer& prevlayer)
{
	if (IsAdjustingBalance(entity, record, &curlayer))
	{
		if (IsAdjustingBalance(entity, record, &prevlayer))
		{
			if ((prevlayer.m_flCycle != curlayer.m_flCycle) || curlayer.m_flWeight == 1.f)
			{
				return true;
			}
		}
	}
	return false;
}

bool CResolver::ABFakeHead(CBaseEntity* entity, ResolveInfo &record, AnimationLayer& curlayer)
{
	if (IsAdjustingBalance(entity, record, &curlayer))
	{
		if (fabsf(GetLbyDelta(record)) > 35.f)
		{
			return true;
		}
	}
	return false;
}

