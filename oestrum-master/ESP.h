#pragma once

class CESP
{
public:
	void DrawPlayer(CBaseEntity* pPlayer, CBaseEntity* pLocalPlayer);
	void DrawBones(CBaseEntity* pBaseEntity);
	void SpreadCrosshair();
	void DrawAngleLines();
	void DrawAngleArrows();
	void DrawScope(CBaseEntity* pLocalPlayer);
	void DrawPlantedBomb(CBaseEntity * pBomb, bool is_planted, CBaseEntity * pLocalPlayer);
	void DrawNades(CBaseEntity * pThrowable);
	void DrawDroppedWeapon(CBaseCombatWeapon * pWeapon);
	void Run();

}; extern CESP* g_ESP;