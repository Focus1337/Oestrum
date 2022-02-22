#include "sdk.h"
#include "global.h"
#include "Knifebot.h"
#include "Math.h"

float attack1 = 75.f;
float attack2 = 64.f;
bool Knifebot::knifing = false;
bool backstabable;
int knifeAction = 0; // 0 = Nothing, 1 = Left Knife, 2 = Right Knife

void Knifebot::Run(CUserCmd* cmd)
{
	Knifebot::knifing = false;

	if (!g_Settings->misc_KbEnabled)
		return;

	if (!Global::LocalPlayer || !Global::LocalPlayer->IsAlive())
		return;

	CBaseCombatWeapon* activeWeapon = Global::LocalPlayer->GetWeapon();

	if (!activeWeapon)
		return;

	Vector pVecTarget = Global::LocalPlayer->GetEyePosition();
	Vector pVecPos = Global::LocalPlayer->GetAbsOrigin();

	if (activeWeapon->IsKnife() && activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::TASER)
	{
		float bestDist = 999999999.f;

		int bestAction = 0;

		for (int i = 1; i < g_pEngine->GetMaxClients(); i++)
		{
			CBaseEntity* enemy = (CBaseEntity*)g_pEntitylist->GetClientEntity(i);

			if (!enemy
				|| enemy == Global::LocalPlayer
				|| enemy->IsDormant()
				|| !enemy->IsAlive()
				|| enemy->IsProtected()
				|| enemy->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
				continue;


			knifeAction = 0;

			Vector eVecTarget;

			if (cmd->buttons & IN_DUCK)
			{
				eVecTarget = enemy->GetOrigin();
			}

			else
			{
				eVecTarget = enemy->GetEyePosition();
			};


			float distance;

			Vector distVec;

			if (*Global::LocalPlayer->GetFlags() & FL_ONGROUND) //When on Ground
			{
				distVec = pVecTarget - eVecTarget; //Use Distance from Localplayer Eyes to Enemy Eyes
			}

			else
			{
				distVec = pVecPos - eVecTarget; //Use Distance from Localplayer Origin to Enemy Eyes
			}

			auto m_vecMins = enemy->GetCollideable()->OBBMins();
			auto m_vecMaxs = enemy->GetCollideable()->OBBMaxs();

			//If our bbmax height is in the range of enemy's bbmin and bbmax height, distance equals the 2D one, else it's equals to 2D vector + height difference to the closest enemy's bb height
			float heightDiff = pVecTarget.z - enemy->GetOrigin().z;
			if (heightDiff > 0 && heightDiff < m_vecMaxs.z - m_vecMins.z)
			{
				distance = distVec.Length2D();
				eVecTarget.z = pVecTarget.z;
			}
			else if (heightDiff < 0) {
				distVec.z = heightDiff;
				distance = distVec.Length();
				eVecTarget = enemy->GetOrigin();
			}
			else {
				distVec.z = heightDiff - m_vecMaxs.z - m_vecMins.z;
				distance = distVec.Length();
				eVecTarget = enemy->GetOrigin();
				eVecTarget.z += m_vecMaxs.z - m_vecMins.z;
			}

			int hp = enemy->GetHealth();
			int armor = enemy->GetArmor();

			if (bestDist > distance)
			{
				auto m_vecAngles = enemy->GetAngleRotation();

				Vector vecLOS = (enemy->GetOrigin() - Global::LocalPlayer->GetAbsOrigin());
				vecLOS.NormalizeInPlace();

				Vector vTargetForward;
				Math::AngleVectors(m_vecAngles, &vTargetForward);

				if (vecLOS.Dot(vTargetForward) > 0.475)
				{
					backstabable = true;
				}
				else
				{
					backstabable = false;
				}

				if (!g_Settings->misc_KbBackOnly)
				{
					if (hp > 50 && distance <= attack1)
					{
						knifeAction = 1;
					}

					if (hp <= 50 && distance <= attack2)
					{
						knifeAction = 2;
					}
				}

				if (backstabable && distance <= attack2)
				{
					knifeAction = 2;
				}

				if (bestAction > knifeAction)
					continue;

				//=== Set Angles and Attack

				if (knifeAction == 1 || knifeAction == 2)
				{
					QAngle angle;
					angle = Math::CalcAngle(pVecTarget, eVecTarget);
					Math::NormalizeAngles(angle);
					Math::ClampAngles(angle);
					cmd->viewangles = angle;
				}

				if (knifeAction == 1)
				{
					Knifebot::knifing = true;
					cmd->buttons |= IN_ATTACK;
					if (g_Settings->misc_KbCrouch)
						cmd->buttons |= IN_DUCK;
				}

				if (hp <= 50 && distance <= attack2 && armor == 0)
				{
					knifeAction = 2;
				}

				bestAction = knifeAction;

				if (knifeAction == 2)
				{
					Knifebot::knifing = true;
					cmd->buttons |= IN_ATTACK2;
					if (g_Settings->misc_KbCrouch)
						cmd->buttons |= IN_DUCK;
				}

				bestDist = distance;

			}
		}
	}
}