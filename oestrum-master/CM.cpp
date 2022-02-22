#include "hooks.h"
#include "global.h"
#include "Misc.h"
#include "Math.h"
#include "GameUtils.h"
#include "Aimbot.h"
#include "PredictionSystem.h"
#include "Antiaim.h"
#include "GrenadePrediction.h"
#include "INetChannel.h"
#include "Knifebot.h"

static CPredictionSystem* Prediction = new CPredictionSystem();

static int ground_tick;

void ground_ticks()
{
	if (!Global::LocalPlayer)
		return;

	if (*Global::LocalPlayer->GetFlags() & FL_ONGROUND)
		ground_tick++;
	else
		ground_tick = 0;
}

bool __fastcall Hooks::CreateMove(void* thisptr, void*, float flInputSampleTime, CUserCmd* cmd)
{
	if (cmd)
	{
		if (!cmd->command_number)
			return true;

		g_pEngine->SetViewAngles(cmd->viewangles);
		auto& antiaim = CAntiaim::Get();

		CBaseEntity* pLocalPlayer = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());
		if (pLocalPlayer)
		{
			Global::LocalPlayer = pLocalPlayer;
			Global::pCmd = cmd;

			if (pLocalPlayer->GetHealth() > 0)
			{
				CBaseCombatWeapon* pWeapon = pLocalPlayer->GetWeapon();
				if (pWeapon)
				{
					PVOID pebp;
					__asm mov pebp, ebp;
					bool* pbSendPacket = (bool*)(*(PDWORD)pebp - 0x1C);
					bool& bSendPacket = *pbSendPacket;

					Global::vecUnpredictedVel = Global::LocalPlayer->GetVelocity();
					Global::MainWeapon = pWeapon;
					Global::WeaponData = pWeapon->GetCSWpnData();
					Global::StrafeAngle = Global::pCmd->viewangles;

					static bool chokePacket = false;
					static bool cancelChoke = false;
					static int chokeCount = 0;

					bool IsLadder = pLocalPlayer->GetMoveType() == MOVETYPE_LADDER;

					g_Aimbot->fired = false;
					g_Aimbot->bestdist = 99999.f;

					g_Misc->ClanTag();
					g_Misc->Bunnyhop();
					g_Misc->AutoStrafe();

					if ((cmd->buttons & IN_ATTACK || (cmd->buttons & IN_ATTACK2 && (Global::MainWeapon->WeaponID() == 64 ||
						Global::MainWeapon->IsKnife()))) && GameUtils::IsAbleToShoot())
						g_Aimbot->fired = true;

					Prediction->EnginePrediction(cmd);

					g_Aimbot->Run(cmd);
					g_Aimbot->RemoveRecoil(cmd);

					if (!cancelChoke)
					{
						g_Misc->FakelagsStart(chokePacket);
					}
					else
					{
						cancelChoke = false;
					}

					if (!g_Settings->rage_LagsEnabled && !g_Settings->aa_Enabled)
						chokePacket = false;

					antiaim.choke = chokePacket;

					if (!IsLadder)
						antiaim.Run(cmd);
					ground_ticks();

					if ((cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2 && Global::MainWeapon->WeaponID() == 64) && (Global::MainWeapon->IsPistol()
						|| Global::MainWeapon->WeaponID() == 9 || Global::MainWeapon->WeaponID() == 40))
					{
						static bool bFlip = false;

						if (bFlip)
						{
							cmd->buttons &= ~IN_ATTACK;
						}
						bFlip = !bFlip;
					}

					g_Misc->FakelagsEnd(chokePacket);

					Knifebot::Run(cmd);

					grenade_prediction::Get().Tick(cmd->buttons);

					Global::ForceRealAA = false;
					if (Global::ChokedPackets >= 14)// breaks fakewalk
					{
						Global::SendPacket = true;
						Global::ChokedPackets = 0;
						Global::ForceRealAA = true;
					}

					CBaseAnimState* animstate = Global::LocalPlayer->GetAnimState();

					if (Global::SendPacket)
					{
						if (g_Settings->misc_TPangles == 0)
						{
							if (animstate && animstate->m_bInHitGroundAnimation && ground_tick > 1)
								Global::AAAngle = QAngle(0.0f, Global::pCmd->viewangles.y, Global::pCmd->viewangles.z);
							else
								Global::AAAngle = QAngle(Global::pCmd->viewangles.x, Global::pCmd->viewangles.y, Global::pCmd->viewangles.z);
						}

						Global::ChokedPackets = 0;
						Global::FakeAngle = Global::pCmd->viewangles;
					}
					else
					{
						if (g_Settings->misc_TPangles == 1)
						{
							if (animstate && animstate->m_bInHitGroundAnimation && ground_tick > 1)
								Global::AAAngle = QAngle(0.0f, Global::pCmd->viewangles.y, Global::pCmd->viewangles.z);
							else
								Global::AAAngle = QAngle(Global::pCmd->viewangles.x, Global::pCmd->viewangles.y, Global::pCmd->viewangles.z);
						}

						Global::ChokedPackets++;
						Global::RealAngle = Global::pCmd->viewangles;
					}
					if (g_Settings->misc_TPangles == 2)
					{
						if (animstate && animstate->m_bInHitGroundAnimation && ground_tick > 1)
							Global::AAAngle = QAngle(0.0f, Global::LocalPlayer->LowerBodyYaw(), Global::pCmd->viewangles.z);
						else
							Global::AAAngle = QAngle(Global::pCmd->viewangles.x, Global::LocalPlayer->LowerBodyYaw(), Global::pCmd->viewangles.z);
					}

					Global::SendPacket = !chokePacket;
					chokePacket = !chokePacket;

					g_Misc->FixMovement();
					g_Misc->FixCmd();

					cmd = Global::pCmd;
					bSendPacket = Global::SendPacket;
					grenade_prediction::Get().Tick(Global::pCmd->buttons);
				}
			}
		}
	}
	return false;
}