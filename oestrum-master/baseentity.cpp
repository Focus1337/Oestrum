#include "sdk.h"
#include "Math.h"
#include "global.h"
#include "GameUtils.h"
#include "xor.h"

CBaseCombatWeapon* CBaseEntity::GetWeapon()
{
	ULONG WeaponUlong = *(PULONG)((DWORD)this + offs.m_hActiveWeapon); // hActiveWeapon
	return (CBaseCombatWeapon*)(g_pEntitylist->GetClientEntityFromHandle(WeaponUlong));
}

DWORD GetCSWpnDataAddr;


void CBaseEntity::SetCurrentCommand(CUserCmd *cmd)
{
	static int offset = g_pNetVars->GetOffset("DT_BasePlayer", "m_hConstraintEntity");
	*Member<CUserCmd**>(this, (offset - 0xC)) = cmd;
}

Vector CBaseEntity::GetWorldSpaceCenter()
{
	Vector max = this->GetMaxs() + this->GetAbsOrigin();
	Vector min = this->GetMins() + this->GetAbsOrigin();

	Vector size = max - min;

	size /= 2;
	size += min;

	return size;
}

bool CBaseEntity::IsValidRenderable()
{

	if (!this || this == nullptr || Global::LocalPlayer == nullptr)
		return false;

	if (this == Global::LocalPlayer)
		return false;

	if (this->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
		return false;

	if (this->IsDormant())
		return false;

	if (!this->IsAlive())
		return false;

	return true;
}

bool CBaseEntity::IsValidTarget()
{
	if (!this || this == nullptr)
		return false;

	ClientClass* pClass = (ClientClass*)this->GetClientClass(); // Needed to check clientclass after nullptr check that was causing a crash

	if (this == Global::LocalPlayer)
		return false;

	if (pClass->m_ClassID != 35)
		return false;

	if (this->GetTeamNum() == Global::LocalPlayer->GetTeamNum())
		return false;

	if (this->IsDormant())
		return false;

	if (!this->IsAlive())
		return false;

	if (this->IsProtected())
		return false;

	return true;
}

Vector CBaseEntity::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones2(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_pModelInfo->GetStudioModel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->pHitboxSet(0)->pHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				GameUtils::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				GameUtils::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}

bool CBaseEntity::GetHitboxPos(int hitbox, Vector &output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t *model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t *studioHdr = g_pModelInfo->GetStudioModel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones2(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t *studioBox = studioHdr->pHitboxSet(0)->pHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	GameUtils::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	GameUtils::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}


void CBaseEntity::SetAbsOrigin(const Vector &origin)
{
	typedef void(__thiscall* oGetServrAngles)(void*, const Vector& origin);
	static oGetServrAngles SetAbsOrigin;

	if (!SetAbsOrigin)
		SetAbsOrigin = reinterpret_cast<oGetServrAngles>(FindPatternIDA("client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1"));

	SetAbsOrigin(this, origin);
}

void CBaseEntity::SetAngle2(Vector wantedang)
{
	typedef void(__thiscall* SetAngleFn)(void*, const Vector &);
	static SetAngleFn SetAngle = (SetAngleFn)((DWORD)Utilities::Memory::FindPatternIDA("client.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
	SetAngle(this, wantedang);
}


Vector CBaseEntity::GetBonePos(int i)
{
	matrix3x4_t boneMatrix[128];
	if (this->SetupBones2(boneMatrix, 128, BONE_USED_BY_HITBOX, g_pGlobals->curtime))
	{
		return Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
	}
	return Vector(0, 0, 0);
}

CSWeaponInfo* CBaseCombatWeapon::GetCSWpnData()
{
	if (!this || this == NULL)
		return NULL;

	typedef CSWeaponInfo *(__thiscall *o_getWeapInfo)(void*);
	return getvfunc<o_getWeapInfo>(this, 445)(this);
}
#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / g_pGlobals->interval_per_tick ) )

bool CBaseEntity::SetupBones2(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	__asm
	{
		mov edi, this
		lea ecx, dword ptr ds : [edi + 0x4]
		mov edx, dword ptr ds : [ecx]
		push currentTime
		push boneMask
		push nMaxBones
		push pBoneToWorldOut
		call dword ptr ds : [edx + 0x34]
	}
}

bool CBaseEntity::IsPlayer()
{
	ClientClass* pClass = (ClientClass*)this->GetClientClass();
	return pClass->m_ClassID == 35;
}

bool CBaseEntity::IsWeapon()
{
	return getvfunc<bool(__thiscall*)(CBaseEntity*)>(this, 160)(this);
}

float &CBaseEntity::GetMaxSpeed()
{
	return *(float_t*)((uintptr_t)this + 0x322C);
}

float &CBaseEntity::GetSurfaceFriction()
{
	static unsigned int _m_surfaceFriction = g_pData->Find(GetPredDescMap(), "m_surfaceFriction");
	return *(float_t*)((uintptr_t)this + _m_surfaceFriction);
}

void CBaseEntity::InvalidateBoneCache()
{
	unsigned long g_iModelBoneCounter = **(unsigned long**)(offs.invalidateBoneCache + 10);
	*(unsigned int*)((DWORD)this + 0x2914) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2680) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

int CBaseEntity::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x297C);
}

AnimationLayer *CBaseEntity::GetAnimOverlays()
{
	return *(AnimationLayer**)((DWORD)this + 10608);
}

AnimationLayer& CBaseEntity::GetAnimOverlay(int Index)
{
	return (*(AnimationLayer**)((DWORD)this + 0x2970))[Index];
}

int CBaseEntity::GetSequenceActivity(int sequence)
{
	auto hdr = g_pModelInfo->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(offs.getSequenceActivity);

	return get_sequence_activity(this, hdr, sequence);
}

CBaseAnimState* CBaseEntity::GetAnimState()
{
	return *reinterpret_cast<CBaseAnimState**>(uintptr_t(this) + 0x3874);
}

void CBaseEntity::SetAbsAngles(const QAngle &angles)
{
	using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle &angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)FindPatternIDA(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8"));
	SetAbsAngles(this, angles);
}

void CBaseEntity::UpdateClientSideAnimation()
{
	typedef void(__thiscall *o_updateClientSideAnimation)(void*);
	getvfunc<o_updateClientSideAnimation>(this, 218)(this);
}

QAngle &CBaseEntity::GetAngleRotation()
{	
	//offs.m_angRotation);
	return *(QAngle*)((uintptr_t)this + 0x128);
}

QAngle &CBaseEntity::GetAbsRotation()
{
	return *(QAngle*)((DWORD)&GetAngleRotation() - 12);
}
float& CBaseEntity::GetFallVelocity()
{
	return *reinterpret_cast<float_t*>((DWORD)this + 0x3004);//offs.m_flFallVelocity);
}