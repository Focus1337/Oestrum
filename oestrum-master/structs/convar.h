#pragma once
typedef void(*FnChangeCallbackV1_t)(void);
typedef void(*FnChangeCallback_t)(void* var, const char *pOldValue, float flOldValue);

struct CVValue_t
{
	char* m_pszString;
	int m_StringLength;

	// Values
	float m_fValue;
	int m_nValue;
};

class ConVar
{
public:
	const char* GetString()
	{
		typedef const char*(__thiscall* GetString_t)(void*);
		return getvfunc<GetString_t>(this, 11)(this);
	}

	float GetFloat()
	{
		typedef float(__thiscall* GetFloat_t)(void*);
		return getvfunc <GetFloat_t>(this, 12)(this);
	}

	int GetInt()
	{
		typedef int(__thiscall* GetInt_t)(void*);
		return getvfunc <GetInt_t>(this, 13)(this);
	}
	void SetValue(const char *value)
	{
		typedef void(__thiscall* SetValue_t)(void*, const char*);
		return getvfunc<SetValue_t>(this, 14)(this, value);
	}

	void SetValue(float value)
	{
		typedef void(__thiscall* SetValue_t)(void*, float);
		return getvfunc<SetValue_t>(this, 15)(this, value);
	}

	void SetValue(int value)
	{
		typedef void(__thiscall* SetValue_t)(void*, int);
		return getvfunc<SetValue_t>(this, 16)(this, value);
	}

	char* GetName()
	{
		typedef char*(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 5)(this);
	}

	
	void* virtualtable;
	ConVar* m_pNext;
	int m_bRegistered;
	char* m_pszName;
	char* m_pszHelpString;
	int m_nFlags;
	FnChangeCallbackV1_t m_fnChangeCallbacksV1;
	ConVar* m_pParent;
	char* m_pszDefaultValue;
	CVValue_t m_Value;
	int m_bHasMin;
	float m_fMinVal;
	int m_bHasMax;
	float m_fMaxVal;
	CUtlVector<FnChangeCallback_t> m_fnChangeCallbacks; // note: this is also accessible as FnChangeCallback_t* instead of CUtlVector
	float GetValue()
	{
		DWORD xored = *(DWORD*)&m_pParent->m_Value.m_fValue ^ (DWORD)this;
		return *(float*)&xored;
	}
	int GetValueN()
	{
		return (int)(m_pParent->m_Value.m_nValue^ (DWORD)this);
	}

};//Size=0x0048*
/*

class IAppSystem
{
public:
	virtual ~IAppSystem()
	{
	}

	virtual void func0() = 0;
	virtual void func1() = 0;
	virtual void func2() = 0;
	virtual void func3() = 0;
	virtual void func4() = 0;
	virtual void func5() = 0;
	virtual void func6() = 0;
	virtual void func7() = 0;
	virtual void func8() = 0;
	virtual void func9() = 0;
};

struct CVarDLLIdentifier_t;

class ICvar : public IAppSystem
{
public:
	virtual void			func10() = 0;
	virtual void			RegisterConCommand(ConVar *pCommandBase) = 0;
	virtual void			UnregisterConCommand(ConVar *pCommandBase) = 0;
	virtual void			func13() = 0;
	virtual ConVar			*FindVar(const char *var_name) = 0;
	virtual void			func15() = 0;
	virtual void			func16() = 0;
	virtual void			func17() = 0;
	virtual void			func18() = 0;
	virtual void			func19() = 0;
	virtual void			func20() = 0;


};

*/