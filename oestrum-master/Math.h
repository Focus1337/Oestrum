#pragma once
namespace Math
{
	extern float NormalizeYaw(float value);
	extern void VectorAngles(const Vector&vecForward, Vector&vecAngles);
	extern void VectorAngles(const Vector & forward, const Vector & pseudoup, QAngle & angles);
	extern void AngleVector(const QAngle & angles, Vector & forward, Vector & right, Vector & up);
	extern void NormalizeAngles(QAngle & angles);
	extern void AngleVectors(const Vector angles, Vector& forward, Vector& right, Vector& up);
	extern void NormalizeVector(Vector & vecIn);
	extern void AngleVectors(const Vector & angles, Vector * forward);
	extern void ClampAngles(QAngle& angles);
	extern float AngleNormalize(float angle);
	extern bool ClampAll(QAngle & angles);
	extern float GetFov(const QAngle & viewAngle, const QAngle & aimAngle);
	extern float RandomFloat(float min, float max);
	extern int RandomSeed(int iSeed);
	extern float RandomFloat2(float min, float max);
	extern void AngleVectors(const Vector &angles, Vector *forward, Vector *right, Vector *up);
	extern QAngle CalcAngle(Vector src, Vector dst);
	extern Vector CalcAngle2(Vector src, Vector dst);
	extern Vector NormalizeAngle(Vector angle);
	extern Vector RotateVectorYaw(Vector origin, float angle, Vector point);

	inline float ClampYaw(float yaw)
	{
		while (yaw > 180.f)
			yaw -= 360.f;
		while (yaw < -180.f)
			yaw += 360.f;
		return yaw;
	}
}

extern bool enabledtp;