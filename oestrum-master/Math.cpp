#include "sdk.h"
#include "Math.h"

float Math::RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
int Math::RandomSeed(int iSeed)
{
	typedef int(__cdecl *RandomSeedFn)(int iSeed);

	static RandomSeedFn oRandomSeed = NULL;

	if (!oRandomSeed)
		oRandomSeed = (RandomSeedFn)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed");

	return oRandomSeed(iSeed);
}

float Math::RandomFloat2(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	return ((RandomFloat_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat")) (min, max);
}

void Math::AngleVectors(const Vector &angles, Vector *forward, Vector *right, Vector *up)
{
	auto SinCos = [](float flRadians, float* pflSine, float* pflCosine)
	{
		__asm
		{
			fld	DWORD PTR[flRadians]
			fsincos
			mov edx, DWORD PTR[pflCosine]
			mov eax, DWORD PTR[pflSine]
			fstp DWORD PTR[edx]
			fstp DWORD PTR[eax]
		}
	};
	float sr, sp, sy, cr, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);

	if (forward)
	{
		forward->x = cp*cy;
		forward->y = cp*sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right->y = (-1 * sr*sp*sy + -1 * cr*cy);
		right->z = -1 * sr*cp;
	}

	if (up)
	{
		up->x = (cr*sp*cy + -sr*-sy);
		up->y = (cr*sp*sy + -sr*cy);
		up->z = cr*cp;
	}
}

QAngle Math::CalcAngle(Vector src, Vector dst)
{
	QAngle angles;
	Vector delta = src - dst;

	if (delta[1] == 0.0f && delta[0] == 0.0f)
	{
		angles[0] = (delta[2] > 0.0f) ? 270.0f : 90.0f; // Pitch (up/down)
		angles[1] = 0.0f;  //yaw left/right
	}
	else
	{
		angles[0] = atan2(-delta[2], delta.Length2D()) * -180 / M_PI;
		angles[1] = atan2(delta[1], delta[0]) * 180 / M_PI;

		if (angles[1] > 90) angles[1] -= 180;
		else if (angles[1] < 90) angles[1] += 180;
		else if (angles[1] == 90) angles[1] = 0;
	}

	angles[2] = 0.0f;
	angles.NormalizeAngles();

	return angles;
}

Vector Math::CalcAngle2(Vector src, Vector dst)
{
	auto VectorAngles2222=[](const Vector& forward, Vector &angles)
	{
		float tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0)
		{
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	};

	Vector ret;
	VectorAngles2222(dst - src, ret);
	return ret;
}

Vector Math::NormalizeAngle(Vector angle)
{
	while (angle.x > 89.f)
	{
		angle.x -= 180.f;
	}
	while (angle.x < -89.f)
	{
		angle.x += 180.f;
	}
	if (angle.y > 180)
	{
		angle.y -= (round(angle.y / 360) * 360.f);
	}
	else if (angle.y < -180)
	{
		angle.y += (round(angle.y / 360) * -360.f);
	}
	if ((angle.z > 50) || (angle.z < 50))
	{
		angle.z = 0;
	}
	return angle;
}

Vector Math::RotateVectorYaw(Vector origin, float angle, Vector point)
{
	float s;
	float c;
	SinCos(DEG2RAD(angle), s, c);

	// translate point back to origin:
	point.x -= origin.x;
	point.y -= origin.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + origin.x;
	point.y = ynew + origin.y;
	return point;
}

float Math::NormalizeYaw(float value)
{
	while (value > 180)
		value -= 360.f;

	while (value < -180)
		value += 360.f;
	return value;
}

void Math::VectorAngles(const Vector&vecForward, Vector&vecAngles)
{
	Vector vecView;
	if (vecForward[1] == 0.f && vecForward[0] == 0.f)
	{
		vecView[0] = 0.f;
		vecView[1] = 0.f;
	}
	else
	{
		vecView[1] = atan2(vecForward[1], vecForward[0]) * 180.f / M_PI;

		if (vecView[1] < 0.f)
			vecView[1] += 360;

		vecView[2] = sqrt(vecForward[0] * vecForward[0] + vecForward[1] * vecForward[1]);

		vecView[0] = atan2(vecForward[2], vecView[2]) * 180.f / M_PI;
	}

	vecAngles[0] = -vecView[0];
	vecAngles[1] = vecView[1];
	vecAngles[2] = 0.f;
}

void Math::VectorAngles(const Vector &forward, const Vector &pseudoup, QAngle &angles)
{
	auto CrossProducts = [](const Vector &a, const Vector &b)
	{
		return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
	};

	Vector left = CrossProducts(pseudoup, forward);
	left.NormalizeInPlace();

	float forwardDist = forward.Length2D();

	if (forwardDist > 0.001f)
	{
		angles.x = atan2f(-forward.z, forwardDist) * 180 / 3.14159265358979323846f;
		angles.y = atan2f(forward.y, forward.x) * 180 / 3.14159265358979323846f;

		float upZ = (left.y * forward.x) - (left.x * forward.y);
		angles.z = atan2f(left.z, upZ) * 180 / 3.14159265358979323846f;
	}
	else
	{
		angles.x = atan2f(-forward.z, forwardDist) * 180 / 3.14159265358979323846f;
		angles.y = atan2f(-left.x, left.y) * 180 / 3.14159265358979323846f;
		angles.z = 0;
	}
}

void Math::AngleVector(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

	SinCos(DEG2RAD(angles[1]), sy, cy);
	SinCos(DEG2RAD(angles[0]), sp, cp);
	SinCos(DEG2RAD(angles[2]), sr, cr);

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr*-sy);
	up.y = (cr * sp * sy + -sr*cy);
	up.z = (cr * cp);
}

void Math::NormalizeAngles(QAngle& angles)
{
	for (auto i = 0; i < 3; i++) {
		while (angles[i] < -180.0f) angles[i] += 360.0f;
		while (angles[i] >  180.0f) angles[i] -= 360.0f;
	}
}

void Math::AngleVectors(const Vector angles, Vector& forward, Vector& right, Vector& up)
{
	float angle;
	static float sp, sy, cp, cy;

	angle = angles[0] * (M_PI / 180.f);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles[1] * (M_PI / 180.f);
	sy = sin(angle);
	cy = cos(angle);


	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;


	static float sr, cr;

	angle = angles[2] * (M_PI / 180.f);
	sr = sin(angle);
	cr = cos(angle);


	right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
	right[1] = -1 * sr * sp * sy + -1 * cr *cy;
	right[2] = -1 * sr * cp;

	up[0] = cr * sp *cy + -sr * -sy;
	up[1] = cr * sp *sy + -sr * cy;
	up[2] = cr * cp;


}

void Math::NormalizeVector(Vector& vecIn) // fuck this cod
{
	for (int axis = 0; axis < 3; ++axis)
	{
		while (vecIn[axis] > 180.f)
			vecIn[axis] -= 360.f;

		while (vecIn[axis] < -180.f)
			vecIn[axis] += 360.f;

	}

	vecIn[2] = 0.f;
}

void SinCosBitch(float a, float* s, float*c)
{
	*s = sin(a);
	*c = cos(a);
}

void Math::AngleVectors(const Vector &angles, Vector *forward)
{
	float	sp, sy, cp, cy;

	auto SinCos1=[](float radians, float *sine, float *cosine)
	{
		*sine = sin(radians);
		*cosine = cos(radians);

	};

	SinCos1(DEG2RAD(angles[1]), &sy, &cy);
	SinCos1(DEG2RAD(angles[0]), &sp, &cp);

	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;
}

void Math::ClampAngles(QAngle& angles)
{
	if (angles.x > 89.f)
		angles.x = 89.f;
	if (angles.x < -89.f)
		angles.x = -89.f;
	if (angles.y > 180.f)
		angles.y = 180.f;
	if (angles.y < -180.f)
		angles.y = -180.f;

	angles.z = 0.f;
}


float Math::AngleNormalize(float angle)
{
	angle = std::fmod(angle, 360.f);

	if (angle > 180.f)
		angle -= 360.f;
	else if (angle < -180.f)
		angle += 360.f;

	return angle;
}

bool Math::ClampAll(QAngle &angles) {
	QAngle a = angles;
	auto ClampAngles1 =[](QAngle &angles)
	{
		if (angles.y > 180.0f)
			angles.y = 180.0f;
		else if (angles.y < -180.0f)
			angles.y = -180.0f;

		if (angles.x > 89.0f)
			angles.x = 89.0f;
		else if (angles.x < -89.0f)
			angles.x = -89.0f;

		angles.z = 0;
	};
	auto Normalize1=[](QAngle& angle)
	{
		while (angle.x > 89.0f) {
			angle.x -= 180.f;
		}
		while (angle.x < -89.0f) {
			angle.x += 180.f;
		}
		while (angle.y > 180.f) {
			angle.y -= 360.f;
		}
		while (angle.y < -180.f) {
			angle.y += 360.f;
		}
	};
	
	Normalize1(a);
	ClampAngles1(a);

	if (isnan(a.x) || isinf(a.x) ||
		isnan(a.y) || isinf(a.y) ||
		isnan(a.z) || isinf(a.z)) {
		return false;
	}
	else {
		angles = a;
		return true;
	}
}

float Math::GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	NormalizeAngles(delta);

	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}