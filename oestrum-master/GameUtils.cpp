#include "hooks.h"
#include "GameUtils.h"
#include "global.h"
#include "Math.h"
bool CGameTrace::DidHitWorld() const
{
    return m_pEnt == g_pEntitylist->GetClientEntity( 0 );
}
bool CGameTrace::DidHit() const
{
    return fraction < 1.0f || allsolid || startsolid;
}

// Returns true if we hit something and it wasn't the world.
bool CGameTrace::DidHitNonWorldCBaseEntity() const
{
    return m_pEnt != NULL && !DidHitWorld();
}

bool GameUtils::WorldToScreen( const Vector& in, Vector& position )
{
    return ( g_pDebugOverlay->ScreenPosition( in, position ) != 1 );
}

Vector GameUtils::GetBonePosition( CBaseEntity* pPlayer, int Bone, VMatrix MatrixArray[ 128 ] )
{
    Vector pos = Vector();

	VMatrix & HitboxMatrix = MatrixArray[ Bone ];

    pos = Vector( HitboxMatrix[ 0 ][ 3 ], HitboxMatrix[ 1 ][ 3 ], HitboxMatrix[ 2 ][ 3 ] );

    return pos;
}


bool GameUtils::IsVisibleScan(CBaseEntity *player)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	Vector eyePos = Global::LocalPlayer->GetEyePosition();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = Global::LocalPlayer;

	if (!player->SetupBones2(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f))
	{
		return false;
	}

	auto studio_model = g_pModelInfo->GetStudioModel(player->GetModel());
	if (!studio_model)
	{
		return false;
	}

	int scan_hitboxes[] = {
		HITBOX_HEAD,
		HITBOX_LEFT_FOREARM,
		HITBOX_LEFT_UPPER_ARM,
		HITBOX_LEFT_FOOT,
		HITBOX_RIGHT_FOOT,
		HITBOX_LEFT_CALF,
		HITBOX_RIGHT_CALF,
		HITBOX_THORAX,
		HITBOX_BELLY
	};

	for (int i = 0; i < ARRAYSIZE(scan_hitboxes); i++)
	{
		auto hitbox = studio_model->pHitboxSet(player->GetHitboxSet())->pHitbox(scan_hitboxes[i]);
		if (hitbox)
		{
			auto
				min = Vector{},
				max = Vector{};

			GameUtils::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
			GameUtils::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

			ray.Init(eyePos, (min + max) * 0.5);
			g_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

			if (tr.m_pEnt == player || tr.fraction > 0.97f)
				return true;
		}
	}
	return false;
}

QAngle GameUtils::CalculateAngle( Vector vecOrigin, Vector vecOther )
{
    auto ret = Vector();
    Vector delta = vecOrigin - vecOther;
    double hyp = delta.Length2D();
    ret.y = ( atan( delta.y / delta.x ) * 57.295779513082f );
    ret.x = ( atan( delta.z / hyp ) * 57.295779513082f );
    ret.z = 0.f;

    if( delta.x >= 0.f )
        ret.y += 180.f;
    return ret;
}

bool GameUtils::IsAbleToShoot()
{
    if( !g_pGlobals )
        return false;

    if( !Global::LocalPlayer || !Global::MainWeapon )
        return false;

    auto flServerTime = (float)Global::LocalPlayer->GetTickBase() * g_pGlobals->interval_per_tick;
    auto flNextPrimaryAttack = Global::MainWeapon->NextPrimaryAttack();

    return( !( flNextPrimaryAttack > flServerTime ) );
}

__forceinline float DotProduct2(const float *a, const float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void VectorTransform(const float *in1, const matrix3x4_t& in2, float *out)
{
	out[0] = DotProduct2(in1, in2[0]) + in2[0][3];
	out[1] = DotProduct2(in1, in2[1]) + in2[1][3];
	out[2] = DotProduct2(in1, in2[2]) + in2[2][3];
}
void GameUtils::VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
{
	VectorTransform(&in1.x, in2, &out.x);
}

std::vector<Vector> GameUtils::HitboxMultipoint(CBaseEntity* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128])
{
	auto VectorTransform_Wrapper = [](const Vector& in1, const matrix3x4_t &in2, Vector &out)
	{
		auto VectorTransform = [](const float *in1, const matrix3x4_t& in2, float *out)
		{
			auto DotProducts = [](const float *v1, const float *v2)
			{
				return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
			};
			out[0] = DotProducts(in1, in2[0]) + in2[0][3];
			out[1] = DotProducts(in1, in2[1]) + in2[1][3];
			out[2] = DotProducts(in1, in2[2]) + in2[2][3];
		};
		VectorTransform(&in1.x, in2, &out.x);
	};

	studiohdr_t* pStudioModel = g_pModelInfo->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);
	mstudiobbox_t *hitbox = set->pHitbox(iHitbox);

	std::vector<Vector> vecArray;

	Vector max;
	Vector min;
	VectorTransform_Wrapper(hitbox->bbmax, BoneMatrix[hitbox->bone], max);
	VectorTransform_Wrapper(hitbox->bbmin, BoneMatrix[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	QAngle CurrentAngles = GameUtils::CalculateAngle(center, Global::LocalPlayer->GetEyePosition());

	Vector Forward;
	Math::AngleVectors(CurrentAngles, &Forward);

	Vector Right = Forward.Cross(Vector(0, 0, 1));
	Vector Left = Vector(-Right.x, -Right.y, Right.z);

	Vector Top = Vector(0, 0, 1);
	Vector Bot = Vector(0, 0, -1);

	const float HEAD_SCALE = g_Settings->rage_MultipHead;
	const float BODY_SCALE = g_Settings->rage_MultipBody;

	switch (iHitbox)
	{
	case 0:
	{
		for (auto i = 0; i < 4; ++i)
		{
			vecArray.emplace_back(center);
		}
		vecArray[1] += Top * (hitbox->radius * HEAD_SCALE);
		vecArray[2] += Right * (hitbox->radius * HEAD_SCALE);
		vecArray[3] += Left * (hitbox->radius * HEAD_SCALE);
		break;
	}
	default:
	{
		for (auto i = 0; i < 3; ++i)
		{
			vecArray.emplace_back(center);
		}
		vecArray[1] += Right * (hitbox->radius * BODY_SCALE);
		vecArray[2] += Left * (hitbox->radius * BODY_SCALE);
		break;
	}
	}
	return vecArray;
}

float GameUtils::GetFov(QAngle qAngles, Vector vecSource, Vector vecDestination, bool bDistanceBased)
{
	auto MakeVector = [](QAngle qAngles)
	{
		auto ret = Vector();
		auto pitch = float(qAngles[0] * M_PI / 180.f);
		auto yaw = float(qAngles[1] * M_PI / 180.f);
		auto tmp = float(cos(pitch));
		ret.x = float(-tmp * -cos(yaw));
		ret.y = float(sin(yaw)*tmp);
		ret.z = float(-sin(pitch));
		return ret;
	};

	Vector ang, aim;
	double fov;

	ang = CalculateAngle(vecSource, vecDestination);
	aim = MakeVector(qAngles);
	ang = MakeVector(ang);

	auto mag_s = sqrt((aim[0] * aim[0]) + (aim[1] * aim[1]) + (aim[2] * aim[2]));
	auto mag_d = sqrt((aim[0] * aim[0]) + (aim[1] * aim[1]) + (aim[2] * aim[2]));
	auto u_dot_v = aim[0] * ang[0] + aim[1] * ang[1] + aim[2] * ang[2];

	fov = acos(u_dot_v / (mag_s*mag_d)) * (180.f / M_PI);

	if (bDistanceBased) {
		fov *= 1.4;
		float xDist = abs(vecSource[0] - vecDestination[0]);
		float yDist = abs(vecSource[1] - vecDestination[1]);
		float Distance = sqrt((xDist * xDist) + (yDist * yDist));

		Distance /= 650.f;

		if (Distance < 0.7f)
			Distance = 0.7f;

		if (Distance > 6.5)
			Distance = 6.5;

		fov *= Distance;
	}

	return (float)fov;
}