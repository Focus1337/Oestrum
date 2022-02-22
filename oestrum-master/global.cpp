#include "sdk.h"
#include "global.h"
#include "Draw.h"
#include "GameUtils.h"


CDataMapUtils* g_pData = new CDataMapUtils;
CBaseEntity* Global::LocalPlayer = nullptr;
CBaseCombatWeapon* Global::MainWeapon = nullptr;
CSWeaponInfo* Global::WeaponData = nullptr;
CUserCmd* Global::pCmd = nullptr;
QAngle Global::LastAngle = QAngle();
QAngle Global::AAAngle = QAngle();
QAngle Global::StrafeAngle = QAngle();
QAngle Global::RealAngle = QAngle();
QAngle Global::FakeAngle = QAngle();
bool Global::Return = true;
bool Global::ForceRealAA = false;
bool Global::SendPacket = true;
bool Global::ShowMenu = false;
bool Global::Opened = false;
bool Global::Init = false;
bool Global::bShouldChoke = false;
int Global::ChokedPackets = 0;
int Global::DamageDealt;
int Global::nChockedTicks = 0;
float Global::CurrTime;
HWND Global::Window = nullptr;
CScreen Global::Screen = CScreen();
std::vector<FloatingText> Global::DamageHit;
std::vector<Vector> Global::walkpoints;
Vector Global::vecUnpredictedVel = Vector(0, 0, 0);


itemTimer::itemTimer() {
	maxTime = 0;
}

itemTimer::itemTimer(float _maxTime) {
	maxTime = _maxTime;
}

float itemTimer::getTimeRemaining() {
	auto time = (timeStarted - (float(clock()) / float(CLOCKS_PER_SEC))) + maxTime;
	return time ? time : 0.00001f;
}

float itemTimer::getTimeRemainingRatio() {
	return getTimeRemaining() / getMaxTime();
}

float itemTimer::getMaxTime() {
	return maxTime ? maxTime : 1; 
}

void itemTimer::startTimer() {
	timeStarted = float(float(clock()) / float(CLOCKS_PER_SEC));
}

void itemTimer::setMaxTime(float time) {
	maxTime = time;
}

FloatingText::FloatingText(CBaseEntity* attachEnt, float lifetime, int Damage)
{
	TimeCreated = Global::CurrTime;
	ExpireTime = TimeCreated + lifetime;
	pEnt = attachEnt;
	DamageAmt = Damage;
}

void FloatingText::Draw()
{
	float Red, Green, Blue;
	Red = g_Settings->color_DamageIndicator[0] * 255;
	Green = g_Settings->color_DamageIndicator[1] * 255;
	Blue = g_Settings->color_DamageIndicator[2] * 255;
	auto head = pEnt->GetBonePos(8);
	Vector screen;

	if (GameUtils::WorldToScreen(head, screen))
	{
		auto lifetime = ExpireTime - TimeCreated;
		auto pct = 1 - ((ExpireTime - Global::CurrTime) / lifetime);
		int offset = pct * 50;
		int y = screen.y - 15 - offset;

		if (DamageAmt >= 100)
		{
			g_Draw.StringA(g_Draw.font_esp, true, screen.x, y, Red, Green, Blue, 255, "Critical damage!");
		}
		else
		{
			g_Draw.StringA(g_Draw.font_esp, true, screen.x, y, Red, Green, Blue, 255, "-%i", DamageAmt);
		}
	}
}

int CDataMapUtils::Find(datamap_t* pMap, const char* szName) {
	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == NULL)
				continue;

			if (strcmp(szName, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

			if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = Find(pMap->dataDesc[i].td, szName)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}