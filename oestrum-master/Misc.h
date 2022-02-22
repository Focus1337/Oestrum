#pragma once

class CMisc
{
public:
	void Bunnyhop();
	void AutoStrafe();
	void FakelagsStart(bool & chokePacket);
	void FakelagsEnd(bool chokePacket);
	void ClanTag();
	void FixCmd();
	void FixMovement();
	float m_circle_yaw = 0;
}; extern CMisc* g_Misc;

