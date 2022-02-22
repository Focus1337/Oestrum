#include "sdk.h"
#include "Misc.h"
#include "global.h"
#include <chrono>
#include "GameUtils.h"
#include "Math.h"
#include "xor.h"
#include "Antiaim.h"

CMisc* g_Misc = new CMisc;

void CMisc::Bunnyhop()
{
	if (g_Settings->misc_Bhop)
	{
		static auto bJumped = false;
		static auto bFake = false;
		if (!bJumped && bFake)
		{
			bFake = false;
			Global::pCmd->buttons |= IN_JUMP;
		}
		else if (Global::pCmd->buttons & IN_JUMP)
		{
			if (*Global::LocalPlayer->GetFlags() & FL_ONGROUND)
			{
				bJumped = true;
				bFake = true;
			}
			else
			{
				Global::pCmd->buttons &= ~IN_JUMP;
				bJumped = false;
			}
		}
		else
		{
			bJumped = false;
			bFake = false;
		}
	}
}

float RightMovement;
bool IsActive;
float StrafeAngle;

static float GetTraceFractionWorldProps(Vector start, Vector end)
{
	Ray_t ray;
	trace_t tr;
	CTraceFilterWorldAndPropsOnly filter;

	ray.Init(start, end);
	g_pEngineTrace->TraceRay_NEW(ray, MASK_SOLID, &filter, &tr);

	return tr.fraction;
}

void Start(CUserCmd* pCmd)
{
	StrafeAngle = 0;
	IsActive = true;

	QAngle CurrentAngles;
	CurrentAngles.y = 0;
	CurrentAngles.x = 0;
	Vector Forward = CurrentAngles.Direction();
	Vector Right = Forward.Cross(Vector(0, 0, 1));
	Vector Left = Vector(-Right.x, -Right.y, Right.z);

	float LeftPath = GetTraceFractionWorldProps(Global::LocalPlayer->GetAbsOrigin() + Vector(0, 0, 10), Global::LocalPlayer->GetAbsOrigin() + Left * 450.f + Vector(0, 0, 10));
	float RightPath = GetTraceFractionWorldProps(Global::LocalPlayer->GetAbsOrigin() + Vector(0, 0, 10), Global::LocalPlayer->GetAbsOrigin() + Right * 450.f + Vector(0, 0, 10));

	RightMovement = 1;
}

void Strafe(CUserCmd* pCmd)
{
	Vector Velocity = Global::LocalPlayer->GetVelocity();
	Velocity.z = 0;
	float Speed = Velocity.Length();
	if (Speed < 45) Speed = 45;
	if (Speed > 750) Speed = 750;

	float FinalPath = GetTraceFractionWorldProps(Global::LocalPlayer->GetAbsOrigin() + Vector(0, 0, 10), Global::LocalPlayer->GetAbsOrigin() + Vector(0, 0, 10) + Velocity / 2.0f);
	float DeltaAngle = RightMovement * fmax((275.0f / Speed) * (2.0f / FinalPath) * (128.0f / (1.7f / g_pGlobals->interval_per_tick)) * g_Settings->misc_Retrack, 2.0f);
	StrafeAngle += DeltaAngle;

	if (fabs(StrafeAngle) >= 360.0f)
	{
		StrafeAngle = 0.0f;
		IsActive = false;
	}
	else
	{
		pCmd->forwardmove = cos((StrafeAngle + 90 * RightMovement) * (M_PI / 180.0f)) * 450.f;
		pCmd->sidemove = sin((StrafeAngle + 90 * RightMovement) * (M_PI / 180.0f)) * 450.f;
	}
}


template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
void CMisc::AutoStrafe()
{
	bool CircleStrafe = GetAsyncKeyState(g_Settings->misc_PrespeedKey);

	int MoveType = Global::LocalPlayer->GetMoveType();

	if (!IsActive && CircleStrafe && !(MoveType == 8 || MoveType == 9))
		Start(Global::pCmd);

	if (CircleStrafe && IsActive)
	{
		Strafe(Global::pCmd);
		return;
	}
	else if (IsActive)
		IsActive = false;

	if (!g_Settings->misc_AutoStrafe)
		return;

	if (GetAsyncKeyState(VK_SPACE))
	{
		/* AW*/
		static int old_yaw;

		auto get_velocity_degree = [](float length_2d)
		{
			auto tmp = RAD2DEG(atan(30.f / length_2d));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.f)
				return 90.f;

			else if (tmp < 0.f)
				return 0.f;

			else
				return tmp;
		};

		//if (pLocal->GetMoveType() != MOVETYPE_WALK)
		//return;

		auto velocity = Global::LocalPlayer->GetVelocity();
		velocity.z = 0;

		static auto flip = false;
		auto turn_direction_modifier = (flip) ? 1.f : -1.f;
		flip = !flip;

		if (*Global::LocalPlayer->GetFlags() & FL_ONGROUND || Global::LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
			return;

		if (Global::pCmd->forwardmove > 0.f)
			Global::pCmd->forwardmove = 0.f;

		auto velocity_length_2d = velocity.Length2D();

		auto strafe_angle = RAD2DEG(atan(15.f / velocity_length_2d));

		if (strafe_angle > 90.f)
			strafe_angle = 90.f;

		else if (strafe_angle < 0.f)
			strafe_angle = 0.f;

		Vector Buffer(0, Global::StrafeAngle.y - old_yaw, 0);
		Buffer.y = Math::NormalizeYaw(Buffer.y);

		int yaw_delta = Buffer.y;
		old_yaw = Global::StrafeAngle.y;

		if (yaw_delta > 0.f)
			Global::pCmd->sidemove = -450.f;

		else if (yaw_delta < 0.f)
			Global::pCmd->sidemove = 450.f;

		auto abs_yaw_delta = abs(yaw_delta);

		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.f)
		{
			Vector velocity_angles;
			Math::VectorAngles(velocity, velocity_angles);

			Buffer = Vector(0, Global::StrafeAngle.y - velocity_angles.y, 0);
			Buffer.y = Math::NormalizeYaw(Buffer.y);
			int velocityangle_yawdelta = Buffer.y;

			auto velocity_degree = get_velocity_degree(velocity_length_2d) * g_Settings->misc_Retrack; // retrack value, for teleporters

			if (velocityangle_yawdelta <= velocity_degree || velocity_length_2d <= 15.f)
			{
				if (-(velocity_degree) <= velocityangle_yawdelta || velocity_length_2d <= 15.f)
				{
					Global::StrafeAngle.y += (strafe_angle * turn_direction_modifier);
					Global::pCmd->sidemove = 450.f * turn_direction_modifier;
				}

				else
				{
					Global::StrafeAngle.y = velocity_angles.y - velocity_degree;
					Global::pCmd->sidemove = 450.f;
				}
			}

			else
			{
				Global::StrafeAngle.y = velocity_angles.y + velocity_degree;
				Global::pCmd->sidemove = -450.f;
			}


		}




		Global::pCmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK);

		if (Global::pCmd->sidemove <= 0.0)
			Global::pCmd->buttons |= IN_MOVELEFT;
		else
			Global::pCmd->buttons |= IN_MOVERIGHT;

		if (Global::pCmd->forwardmove <= 0.0)
			Global::pCmd->buttons |= IN_BACK;
		else
			Global::pCmd->buttons |= IN_FORWARD;

	}
}

#pragma region Fakelags
const int MAX_CHOKE = 15;
const float TELEPORT_DISTANCE = 64.f;
int choked;

struct SendInfo
{
	SendInfo(int _tick, Vector _origin)
	{
		tick = _tick;
		origin = _origin;
	}

	int tick;
	Vector origin;
};

std::vector<SendInfo> lastSendInfo;

void CMisc::FakelagsStart(bool &chokePacket)
{
	CUserCmd *pCmd = Global::pCmd;
	CBaseEntity *pLocal = Global::LocalPlayer;


	float speed = Global::LocalPlayer->GetVelocity().Length();

	int Min = g_Settings->rage_LagsMin;
	int Max = g_Settings->rage_LagsMax;
	int InAir = g_Settings->rage_LagsAir;
	bool Enabled = g_Settings->rage_LagsEnabled;


	if (!Enabled)
	{
		Min = 1;
		Max = 1;
		InAir = 0;
	}

	static bool lastInAir = false;

	bool onGround = pLocal->CheckOnGround();

	if (lastInAir && onGround)
	{
		Min = 14;
		Max = 14;
		InAir = 0;
		Enabled = true;
	}

	lastInAir = !onGround;

	static bool flip = false;
	bool air = false;

	if (InAir > 0)
	{
		if (!onGround)
		{
			air = true;
		}
	}

	if (Enabled)
	{
		int min = Min;
		int max = Max;

		float distPerTick = speed * g_pGlobals->interval_per_tick;
		max = min((int)std::ceilf(TELEPORT_DISTANCE / distPerTick) + 1, MAX_CHOKE - 1);

		max = min(Max, max);

		if (speed < 1.0f)
		{
			max = 1;
			min = 1;
		}

		if (choked >= min && choked >= max)
		{
			chokePacket = false;
			choked = 0;
			flip = !flip;
		}
		else
		{
			chokePacket = true;
			choked++;
		}
	}
}

void CMisc::FakelagsEnd(bool chokePacket)
{
	CUserCmd *pCmd = Global::pCmd;
	CBaseEntity *pLocal = Global::LocalPlayer;

	if (!chokePacket)
	{
		lastSendInfo.insert(lastSendInfo.begin(), SendInfo(g_pGlobals->tickcount, pLocal->GetOrigin()));

		if (lastSendInfo.size() > 11)
			lastSendInfo.pop_back();
	}

	int lagCompensatedTicks = 0;

	if (lastSendInfo.size() != 0)
	{
		Vector curOrigin = lastSendInfo.begin()->origin;

		for (auto it = ++lastSendInfo.begin(); it != lastSendInfo.end(); ++it)
		{
			Vector diff = curOrigin - it->origin;
			curOrigin = it->origin;

			if (diff.Length2D() <= TELEPORT_DISTANCE)
			{
				lagCompensatedTicks++;
			}
			else
			{
				break;
			}
		}
	}
}
#pragma endregion

void SetClanTag(const char *tag)
{
	typedef int(__fastcall *SendClanTagFn)(const char*, const char*);
	static SendClanTagFn SendClanTag = NULL;

	if (!SendClanTag)
		SendClanTag = (SendClanTagFn)FindPatternIDA(XorStr("engine.dll"), XorStr("53 56 57 8B DA 8B F9 FF 15 ?? ?? ?? ??"));

	if (SendClanTag)
	{
		SendClanTag(tag, "");
	}
}

void CMisc::ClanTag()
{
	int i = 0;
	float serverTime = (float)g_pEngine->GetServerTick() * (g_pGlobals->interval_per_tick * 2);

	std::vector<std::string> anim;
	std::vector<std::string> old;

	if (g_Settings->misc_Clantag)
	{
		anim.push_back(XorStr("        "));
		anim.push_back(XorStr("|       "));anim.push_back(XorStr("        "));
		anim.push_back(XorStr("O       "));
		anim.push_back(XorStr("O|      ")); anim.push_back(XorStr("O       "));
		anim.push_back(XorStr("Oe      "));
		anim.push_back(XorStr("Oe|     ")); anim.push_back(XorStr("Oe      "));
		anim.push_back(XorStr("Oes     "));
		anim.push_back(XorStr("Oes|    ")); anim.push_back(XorStr("Oes     "));
		anim.push_back(XorStr("Oest    "));
		anim.push_back(XorStr("Oest|   ")); anim.push_back(XorStr("Oest    "));
		anim.push_back(XorStr("Oestr   "));
		anim.push_back(XorStr("Oestr|  ")); anim.push_back(XorStr("Oestr   "));
		anim.push_back(XorStr("Oestru  "));
		anim.push_back(XorStr("Oestru| ")); anim.push_back(XorStr("Oestru  "));
		anim.push_back(XorStr("Oestrum ")); anim.push_back(XorStr("Oestrum "));
		anim.push_back(XorStr("Oestrum|")); anim.push_back(XorStr("Oestru| "));
		anim.push_back(XorStr("Oestr|  ")); anim.push_back(XorStr("Oest|   "));
		anim.push_back(XorStr("Oes|    ")); anim.push_back(XorStr("Oe|     "));
		anim.push_back(XorStr("O|      ")); anim.push_back(XorStr("|       "));
	}

	static bool reset = 0;

	if (g_Settings->misc_Clantag)
	{
		if (old != anim)
		{
			i = (int)(serverTime) % (int)anim.size();
			SetClanTag(anim[i].c_str());
			reset = true;
			old = anim;
		}
	}
	else
	{
		if (reset)
		{
			SetClanTag("");
			reset = false;
		}
	}
}


void CMisc::FixCmd()
{
	if (g_Settings->misc_AntiUntrust)
	{

		Global::pCmd->viewangles.y = Math::NormalizeYaw(Global::pCmd->viewangles.y);
		Math::ClampAngles(Global::pCmd->viewangles);

		if (Global::pCmd->forwardmove > 450)
			Global::pCmd->forwardmove = 450;
		if (Global::pCmd->forwardmove < -450)
			Global::pCmd->forwardmove = -450;

		if (Global::pCmd->sidemove > 450)
			Global::pCmd->sidemove = 450;
		if (Global::pCmd->sidemove < -450)
			Global::pCmd->sidemove = -450;
	}
}


void CMisc::FixMovement()
{
	Vector vMove =Vector(Global::pCmd->forwardmove, Global::pCmd->sidemove, 0.0f);
	float flSpeed = vMove.Length();
	Vector qMove;
	Math::VectorAngles(vMove, qMove);
	float normalized = fmod(Global::pCmd->viewangles.y + 180.f, 360.f) - 180.f;
	float normalizedx = fmod(Global::pCmd->viewangles.x + 180.f, 360.f) - 180.f;
	float flYaw = DEG2RAD((normalized - Global::StrafeAngle.y) + qMove.y);

	if (normalizedx >= 90.0f || normalizedx <= -90.0f || (Global::pCmd->viewangles.x >= 90.f && Global::pCmd->viewangles.x <= 200) || Global::pCmd->viewangles.x <= -90)
		Global::pCmd->forwardmove = -cos(flYaw) * flSpeed;
	else 
		Global::pCmd->forwardmove = cos(flYaw) * flSpeed;

	Global::pCmd->sidemove = sin(flYaw) * flSpeed;
}