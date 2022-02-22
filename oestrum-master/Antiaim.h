#pragma once

#include "Singleton.hpp"

class CAntiaim : public Singleton<CAntiaim>
{
public:
	void Run(CUserCmd * usercmd);
	bool choke;
private:
	float RealYawAngles();
	float FakeYawAngles();
	void Fakewalk(CUserCmd * userCMD);
	void Friction(Vector & outVel);
	bool LbyBreaker1(CUserCmd* pcmd);
	bool LbyBreaker2(CUserCmd* pcmd);
	bool LbyBreaker3(CUserCmd* pcmd);
	float get3ddist2(Vector myCoords, Vector enemyCoords)
	{
		return sqrt(
			pow(double(enemyCoords.x - myCoords.x), 2.0) +
			pow(double(enemyCoords.y - myCoords.y), 2.0) +
			pow(double(enemyCoords.z - myCoords.z), 2.0));
	};
	CUserCmd *usercmd = nullptr;
};