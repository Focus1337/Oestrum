#pragma once

namespace GameUtils
{
	extern bool WorldToScreen(const Vector& in, Vector& position);
	extern QAngle CalculateAngle(Vector vecOrigin, Vector vecOther);
	extern Vector GetBonePosition(CBaseEntity * pPlayer, int Bone, VMatrix MatrixArray[128]);
	extern bool IsVisibleScan(CBaseEntity * player);
	extern bool IsAbleToShoot();
	extern void VectorTransform(const Vector & in1, const matrix3x4_t & in2, Vector & out);
	extern std::vector<Vector> HitboxMultipoint(CBaseEntity * pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128]);
	extern float GetFov(QAngle qAngles, Vector vecSource, Vector vecDestination, bool bDistanceBased);
}