#pragma once
#include "sdk.h"
#include <deque>
#include "Singleton.hpp"

struct ResolveInfo
{
	void SaveRecord(CBaseEntity *player)
	{
		rec_oldlby = player->LowerBodyYaw();
		rec_simtime = player->GetSimulationTime();
		rec_velocity = player->GetVelocity().Length2D();
		rec_eyeangles = *player->GetEyeAnglesPtr();

		rec_layercount = player->GetNumAnimOverlays();
		for (int i = 0; i < rec_layercount; i++)
			rec_animlayer[i] = player->GetAnimOverlays()[i];
	}

	bool operator==(const ResolveInfo &other)
	{
		return other.rec_simtime == rec_simtime;
	}
	// flag_ = flags; rec_ = recorded things; res_ = resolving method; ms_ = missed shots;
	bool flag_ismoving, flag_isinair, flag_isducking, flag_active;

	bool rec_inoverride;
	float rec_velocity, rec_simtime, rec_oldlby, rec_standingtime, rec_lastmovinglby, rec_lbydelta = 180.f;
	int rec_layercount = 0;
	QAngle rec_eyeangles;
	AnimationLayer rec_animlayer[15];

	int ms_air, ms_blby, ms_fwalk, ms_delta120, ms_delta35, ms_fhead;

	Vector lasthitangle = Vector(0, 0, 0);
	Vector lastsetangle = Vector(0, 0, 0);
	
	Vector res_lby, res_lbydelta, res_lastlby, res_inverselby, res_backtracklby;
};

class CResolver : public Singleton<CResolver>
{
public:
	void UpdateLogs();
	void ResolvePlayers(CBaseEntity* entity);
	ResolveInfo res_info[65];
private:
	bool IsAdjustingBalance(CBaseEntity* entity, ResolveInfo& record, AnimationLayer* layer);
	bool IsAdjustingStopMoving(CBaseEntity* entity, ResolveInfo& record, AnimationLayer* layer);
	bool IsFakewalking(CBaseEntity* entity, ResolveInfo& record);
	bool ABDeltaLess120(CBaseEntity* entity, ResolveInfo& record, AnimationLayer &curlayer, AnimationLayer &prevlayer);
	bool ABDeltaLess35(CBaseEntity* entity, ResolveInfo& record, AnimationLayer &curlayer, AnimationLayer &prevlayer);
	bool ABBreakLby(CBaseEntity* entity, ResolveInfo& record, AnimationLayer &curlayer, AnimationLayer &prevlayer);
	bool ABFakeHead(CBaseEntity* entity, ResolveInfo& record, AnimationLayer &curlayer);
	const inline float GetLbyDelta(const ResolveInfo&v)
	{
		return v.rec_eyeangles.y - v.rec_oldlby;
	}
};