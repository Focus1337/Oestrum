#include "offsets.h"
#include <array>
#include "usercmd.h"
class IClientRenderable;
class IClientNetworkable;
class IClientUnknown;
class IClientThinkable;
struct model_t;
class CBaseCombatWeapon;
#define ACTIVITY_NOT_AVAILABLE		-1

class CollisionProperty
{
public:
	Vector VecMins()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + 0x8);
	}
	Vector VecMaxs()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + 0x14);
	}
};

typedef unsigned short ClientShadowHandle_t;
typedef unsigned short ClientRenderHandle_t;
typedef unsigned short ModelInstanceHandle_t;
typedef unsigned char uint8;
class IClientRenderable
{
public:
	virtual IClientUnknown*            GetIClientUnknown() = 0;
	virtual Vector const&              GetRenderOrigin(void) = 0;
	virtual QAngle const&              GetRenderAngles(void) = 0;
	virtual bool                       ShouldDraw(void) = 0;
	virtual int                        GetRenderFlags(void) = 0; // ERENDERFLAGS_xxx
	virtual void                       Unused(void) const {}
	virtual ClientShadowHandle_t       GetShadowHandle() const = 0;
	virtual ClientRenderHandle_t&      RenderHandle() = 0;
	virtual const model_t*             GetModel() const = 0;
	virtual int                        DrawModel(int flags, const int /*RenderableInstance_t*/ &instance) = 0;
	virtual int                        GetBody() = 0;
	virtual void                       GetColorModulation(float* color) = 0;
	virtual bool                       LODTest() = 0;
	virtual bool                       SetupBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;
	virtual void                       SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights) = 0;
	virtual void                       DoAnimationEvents(void) = 0;
	virtual void* /*IPVSNotify*/       GetPVSNotifyInterface() = 0;
	virtual void                       GetRenderBounds(Vector& mins, Vector& maxs) = 0;
	virtual void                       GetRenderBoundsWorldspace(Vector& mins, Vector& maxs) = 0;
	virtual void                       GetShadowRenderBounds(Vector &mins, Vector &maxs, int /*ShadowType_t*/ shadowType) = 0;
	virtual bool                       ShouldReceiveProjectedTextures(int flags) = 0;
	virtual bool                       GetShadowCastDistance(float *pDist, int /*ShadowType_t*/ shadowType) const = 0;
	virtual bool                       GetShadowCastDirection(Vector *pDirection, int /*ShadowType_t*/ shadowType) const = 0;
	virtual bool                       IsShadowDirty() = 0;
	virtual void                       MarkShadowDirty(bool bDirty) = 0;
	virtual IClientRenderable*         GetShadowParent() = 0;
	virtual IClientRenderable*         FirstShadowChild() = 0;
	virtual IClientRenderable*         NextShadowPeer() = 0;
	virtual int /*ShadowType_t*/       ShadowCastType() = 0;
	virtual void                       CreateModelInstance() = 0;
	virtual ModelInstanceHandle_t      GetModelInstance() = 0;
	virtual const matrix3x4_t&         RenderableToWorldTransform() = 0;
	virtual int                        LookupAttachment(const char *pAttachmentName) = 0;
	virtual   bool                     GetAttachment(int number, Vector &origin, QAngle &angles) = 0;
	virtual bool                       GetAttachment(int number, matrix3x4_t &matrix) = 0;
	virtual float*                     GetRenderClipPlane(void) = 0;
	virtual int                        GetSkin() = 0;
	virtual void                       OnThreadedDrawSetup() = 0;
	virtual bool                       UsesFlexDelayedWeights() = 0;
	virtual void                       RecordToolMessage() = 0;
	virtual bool                       ShouldDrawForSplitScreenUser(int nSlot) = 0;
	virtual uint8                      OverrideAlphaModulation(uint8 nAlpha) = 0;
	virtual uint8                      OverrideShadowAlphaModulation(uint8 nAlpha) = 0;
	const model_t* GetModel() //void*
	{
		typedef const model_t*(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(this, 8)(this);
	}
};

class IClientNetworkable
{
public:
	virtual IClientUnknown*	GetIClientUnknown() = 0;
	virtual void			Release() = 0;
	virtual void*			GetClientClass() = 0;// FOR NETVARS FIND YOURSELF ClientClass* stuffs
	virtual void             NotifyShouldTransmit(int state) = 0;
	virtual void             OnPreDataChanged(int updateType) = 0;
	virtual void             OnDataChanged(int updateType) = 0;
	virtual void             PreDataUpdate(int updateType) = 0;
	virtual void             PostDataUpdate(int updateType) = 0;
	virtual void			unknown();
	virtual bool			IsDormant(void) = 0;
	virtual int				Index(void) const = 0;
	virtual void			ReceiveMessage(int classID/* , bf_read &msg*/) = 0;
	virtual void*			GetDataTableBasePtr() = 0;
	virtual void			SetDestroyedOnRecreateEntities(void) = 0;

};


class IClientUnknown
{
public:
	virtual CollisionProperty*		GetCollideable() = 0;
	virtual IClientNetworkable*	GetClientNetworkable() = 0;
	virtual IClientRenderable*	GetClientRenderable() = 0;
	virtual void*		GetIClientCBaseEntity() = 0;
	virtual void*		GetBaseCBaseEntity() = 0;
	virtual IClientThinkable*	GetClientThinkable() = 0;
};
class IClientThinkable
{
public:
	virtual IClientUnknown*		GetIClientUnknown() = 0;
	virtual void				ClientThink() = 0;
	virtual void*				GetThinkHandle() = 0;
	virtual void				SetThinkHandle(void* hThink) = 0;
	virtual void				Release() = 0;
};

class VarMapEntry_t
{
public:
	unsigned short		type;
	unsigned short		m_bNeedsToInterpolate;	// Set to false when this var doesn't
												// need Interpolate() called on it anymore.
	void				*data;
	void	*watcher;
};

struct VarMapping_t
{
	CUtlVector<VarMapEntry_t>	m_Entries;
	int							m_nInterpolatedEntries;
	float						m_lastInterpolationTime;
};


template <typename T>
__forceinline T Member(void *base, DWORD offset = 0)
{
	return (T)((DWORD)base + offset);
}


struct inputdata_t;
typedef enum _fieldtypes
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

							// NOTE: Use float arrays for local transformations that don't need to be fixed up.
							FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
							FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

							FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
							FIELD_MODELINDEX,		// a model index
							FIELD_MATERIALINDEX,	// a material index (using the material precache string table)

							FIELD_VECTOR2D,			// 2 floats

							FIELD_TYPECOUNT,		// MUST BE LAST
} fieldtype_t;

class ISaveRestoreOps;
//class C_BaseEntity;
class CBaseEntity;
//
// Function prototype for all input handlers.
//
//typedef void (C_BaseEntity::*inputfunc_t)(inputdata_t &data);
typedef void (CBaseEntity::*inputfunc_t)(inputdata_t &data);

struct datamap_t;
class typedescription_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,

	// Must be last
	TD_OFFSET_COUNT,
};

class typedescription_t
{
public:
	int32_t fieldType; //0x0000
	char* fieldName; //0x0004
	int fieldOffset[TD_OFFSET_COUNT]; //0x0008
	int16_t fieldSize_UNKNWN; //0x0010
	int16_t flags_UNKWN; //0x0012
	char pad_0014[12]; //0x0014
	datamap_t* td; //0x0020
	char pad_0024[24]; //0x0024
}; //Size: 0x003C

   //-----------------------------------------------------------------------------
   // Purpose: stores the list of objects in the hierarchy
   //            used to iterate through an object's data descriptions
   //-----------------------------------------------------------------------------
struct datamap_t
{
	typedescription_t    *dataDesc;
	int                    dataNumFields;
	char const            *dataClassName;
	datamap_t            *baseMap;

	bool                chains_validated;
	// Have the "packed" offsets been computed
	bool                packed_offsets_computed;
	int                    packed_size;
};

typedef unsigned long CBaseHandle;


class ICollideable {
public:
	Vector& OBBMins() {
		typedef Vector&(__thiscall *OrigFn)(void*);
		return getvfunc<OrigFn>(this, 1)(this);
	}
	Vector& OBBMaxs() {
		typedef Vector&(__thiscall *OrigFn)(void*);
		return getvfunc<OrigFn>(this, 2)(this);
	}
};

class CBaseAnimState
{
public:
	char pad[3];
	char bUnknown; //0x4
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
};

class CBaseEntity
{
public:

	datamap_t *GetPredDescMap()
	{
		typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
		return getvfunc<o_GetPredDescMap>(this, 17)(this);
	}

	ICollideable* GetCollideable() {
		typedef ICollideable*(__thiscall *OrigFn)(void*);
		return getvfunc<OrigFn>(this, 3)(this);
	}

	bool IsPlayer();
	bool IsWeapon();
	float & GetMaxSpeed();
	float & GetSurfaceFriction();
	void InvalidateBoneCache();
	int GetNumAnimOverlays();
	AnimationLayer * GetAnimOverlays();
	AnimationLayer & GetAnimOverlay(int Index);
	int GetSequenceActivity(int sequence);
	CBaseAnimState * GetAnimState();
	void SetAbsAngles(const QAngle & angles);
	void UpdateClientSideAnimation();
	QAngle & GetAngleRotation();
	QAngle & GetAbsRotation();
	float & GetFallVelocity();

	int GetActiveWeaponIndex()
	{
		return *reinterpret_cast<int*>(uintptr_t(this) + offs.m_hActiveWeapon) & 0xFFF;
	}

	UINT PhysicsSolidMaskForEntity()
	{
		typedef UINT(__thiscall* Fn)(void*);
		return getvfunc<Fn>(this, 148)(this);
	}

	void SetCurrentCommand(CUserCmd * cmd);
	Vector GetWorldSpaceCenter();

	bool IsValidRenderable();
	bool IsValidTarget();
	Vector GetHitboxPos(int hitbox_id);
	bool GetHitboxPos(int hitbox, Vector & output);
	void SetAbsOrigin(const Vector & origin);

	int draw_model(int flags, uint8_t alpha) {
		using fn = int(__thiscall*)(void*, int, uint8_t);
		return getvfunc< fn >(GetRenderable(), 9)(GetRenderable(), flags, alpha);
	}

	void SetAngle2(Vector wantedang);

	UINT* GetWearables()
	{
		return (UINT*)((DWORD)this + offs.m_hMyWearables);
	}

	Vector GetBonePos(int i);

	float LowerBodyYaw()
	{
		return *reinterpret_cast<float*>((DWORD)this + offs.m_flLowerBodyYawTarget);
	}

	Vector GetMins()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + 0x0318 + 0x0320);
	}
	Vector GetMaxs()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + 0x0318 + 0x032C);
	}

	float GetSimulationTime()
	{
		return *reinterpret_cast<float*>((DWORD)this + offs.m_flSimulationTime);
	}

	int &GetTickBase(void)
	{
		return *(int*)((DWORD)this + offs.m_nTickBase);
	}

	int GetHitboxSet()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0x09FC);
	}

	uintptr_t m_dwBoneMatrix()
	{
		return *(uintptr_t*)((uintptr_t)this + 0x2698);
	}

	matrix3x4_t GetBoneMatrix(int BoneID)
	{
		return *reinterpret_cast<matrix3x4_t*>(m_dwBoneMatrix() + 0x30 * BoneID);
	}

	float GetNextAttack()
	{
		return *reinterpret_cast<float*>(uint32_t(this) + 0x2D60);
	}

	int GetTeamNum()
	{
		return *(int*)((uintptr_t)this + offs.m_iTeamNum);
	}
	int GetArmor()
	{
		return *reinterpret_cast<int*>((DWORD)this + offs.m_ArmorValue);
	}
	bool m_bHasHeavyArmor()
	{
		return *(bool*)((DWORD)this + offs.m_bHasHeavyArmor);
	}
	bool HasHelmet()
	{
		return *reinterpret_cast<bool*>((DWORD)this + offs.m_bHasHelmet);
	}
	float GetSpawnTime()
	{
		static auto offset = 0x00A290;
		return *(float*)(this + offset);
	}
	int GetHealth()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0xFC);
	}

	VarMapping_t* GetVarMap()
	{
		return reinterpret_cast<VarMapping_t*>((DWORD)this + 0x24);
	}
	int* GetFlags()
	{
		return (int*)((DWORD)this + offs.m_fFlags);
	}
	int GetShotsFired()
	{
		return *reinterpret_cast<int*>((DWORD)this + offs.m_iShotsFired);
	}
	Vector GetAbsOrigin()
	{
		//return *(Vector*)((uintptr_t)this + 0xA0);
		typedef Vector&(__thiscall* oGetServrAngles)(void*);
		return getvfunc<oGetServrAngles>(this, 10)(this);
	}
	QAngle& GetAbsAngles()
	{
		using OriginalFn = QAngle&(__thiscall*)(void*);
		return getvfunc<OriginalFn>(this, 11)(this);
	}
	bool IsMoving()
	{
		if (this->GetVelocity().Length() > 0.1f)
			return true;
		return false;
	}
	Vector GetEyePosition(void)
	{
		return GetAbsOrigin() + *(Vector*)((DWORD)this + offs.m_vecViewOffset);
	}
	Vector ViewOffset()
	{
		return *(Vector*)((uintptr_t)this + offs.m_vecViewOffset);
	}
	Vector GetOrigin()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + offs.m_vecOrigin);
	}	
	float GetStepSize()
	{
		return *reinterpret_cast<float*>(uintptr_t(this + offs.m_flStepSize));
	}
	Vector GetBaseVelocity()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + 0x011C);//offs.m_vecBaseVelocity);
	}
	
	void* GetClientClass()
	{
		void* Networkable = (void*)(this + 0x8);
		typedef void* (__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(Networkable, 2)(Networkable);
	}
	Vector GetVelocity()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + offs.m_vecVelocity);
	}
	bool IsDormant()
	{
		//return *reinterpret_cast<bool*>((DWORD)this + 0xE9);
		return GetNetworkable()->IsDormant();
	}

	int LifeState()
	{
		return *reinterpret_cast<int*>((DWORD)this + offs.m_lifeState);
	}

	bool IsAlive()
	{
	//	BYTE lifestate = *(BYTE*)((DWORD)this + 0x025B);
	//	return (lifestate == 0);
		return LifeState() == LIFE_ALIVE;
	}

	float* GetFlashDuration()
	{
		return (float*)((uintptr_t)this + offs.m_flFlashDuration);
	}

	int GetIndex()
	{
		return GetNetworkable()->Index();
	}
	IClientNetworkable* GetNetworkable()
	{
		return reinterpret_cast<IClientNetworkable*>((DWORD)this + 0x8);
	}
	bool SetupBones2(matrix3x4_t * pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);

	void SetPoseAngles(float_t yaw, float_t pitch)
	{
		auto& poses = *GetPoseParameter();
		poses[11] = (pitch + 90) / 180;
		poses[2] = (yaw + 180) / 360;
	}

	int GetGroundEntityHandle()
	{ 
		return *(int*)((uintptr_t)this + 0x14C);
	}

	bool CheckOnGround()
	{
		bool onGround1 = false;
		bool onGround2 = (*GetFlags() & FL_ONGROUND);
		bool onGround4 = GetGroundEntityHandle() != -1;
		bool onGround3 = false;

		return onGround1 || onGround2 || onGround3 || onGround4;
	}

	const model_t* GetModel() //void*
	{
		void *pRenderable = (void*)(this + 0x4);
		typedef const model_t*(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(pRenderable, 8)(pRenderable);
	}
	int GetCollisionGroup()
	{
		return *reinterpret_cast<int*>((DWORD)this + offs.m_CollisionGroup); 
		//GetNetVar 
	}
	bool IsBroken()
	{
		return *reinterpret_cast<bool*>((DWORD)this + 0x0A04);
	}
	CollisionProperty* GetCollision()
	{
		return reinterpret_cast<CollisionProperty*>((DWORD)this + 0x0318);
	}
	
	VMatrix& GetCollisionBoundTrans()
	{
		return *reinterpret_cast<VMatrix*>((DWORD)this + 0x0440);
	}
	IClientRenderable* GetRenderable()
	{
		return reinterpret_cast<IClientRenderable*>((DWORD)this + 0x4);
	}

	bool IsScoped()
	{
		return *reinterpret_cast<bool*>((DWORD)this + offs.m_bIsScoped);
	}
	bool IsProtected(void)
	{
		return *(bool*)((DWORD)this + offs.m_bGunGameImmunity);
	}
	QAngle GetPunchAngle()
	{
		//return *reinterpret_cast<QAngle*>((DWORD)this + 0x0000301C);
		return *(QAngle*)((uintptr_t)this + offs.m_aimPunchAngle);
	}
	QAngle* GetPunchAnglePtr()
	{
		return reinterpret_cast<QAngle*>((DWORD)this + 0x301C);
	}
	QAngle GetPunchAngleVelocity()
	{
		return *(QAngle*)((uintptr_t)this + offs.m_aimPunchAngleVel);
	}

	QAngle GetViewPunch()
	{
		return *reinterpret_cast<QAngle*>((DWORD)this + 0x3010);
	}
	QAngle* GetViewPunchPtr()
	{
		return reinterpret_cast<QAngle*>((DWORD)this + 0x3010);
	}
	Vector GetEyeAngles()
	{
		return *reinterpret_cast<Vector*>((DWORD)this + offs.m_angEyeAngles);
	}
	Vector* GetEyeAnglesPtr()
	{
		return reinterpret_cast<Vector*>((DWORD)this + offs.m_angEyeAngles);
	}
	void SetEyeAngles(Vector angles)
	{
		*reinterpret_cast<Vector*>(uintptr_t(this) + offs.m_angEyeAngles) = angles;
	}

	CBaseCombatWeapon* GetWeapon();
	int GetGlowIndex()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0x0000A320);
	}
	UINT* GetWeapons()
	{
		return (UINT*)((DWORD)this + 0x00002DE8);
	}
	int GetMoveType()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0x258);
	}
	int GetMoney()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0xB228);
	}
	int GetDucking()
	{
		return *reinterpret_cast<int*>((DWORD)this + 0x3035);
	}

	std::array<float, 24>* GetPoseParameter()
	{
		return (std::array<float, 24>*)((uintptr_t)this + offs.m_flPoseParameter);
	}

};