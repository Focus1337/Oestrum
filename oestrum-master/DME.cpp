#include "hooks.h"
#include "global.h"
#include "MaterialHelper.h"
#include "xor.h"
#include "GameUtils.h"
#include "Glow.h"

void __fastcall Hooks::scene_end(void* thisptr, void* edx)
{
	static auto scene_end_o = renderviewVMT->GetOriginalMethod< decltype(&scene_end) >(9);
	scene_end_o(thisptr, edx);

	static IMaterial* CoveredLit = g_MaterialHelper->CreateMaterial(true);
	static IMaterial* OpenLit = g_MaterialHelper->CreateMaterial(false);
	static IMaterial* CoveredFlat = g_MaterialHelper->CreateMaterial(true, false);
	static IMaterial* OpenFlat = g_MaterialHelper->CreateMaterial(false, false);

	IMaterial *fakeang_mat = CoveredFlat;
	IMaterial *covered = g_Settings->visuals_ChamsStyle == 0 ? CoveredLit : CoveredFlat;
	IMaterial *open = g_Settings->visuals_ChamsStyle == 0 ? OpenLit : OpenFlat;

	auto b_IsThirdPerson = *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(g_pInput) + 0xA5);

	for (int i = 1; i < g_pEngine->GetMaxClients(); ++i) {
		CBaseEntity* ent = (CBaseEntity*)g_pEntitylist->GetClientEntity(i);

		if (ent == Global::LocalPlayer && Global::LocalPlayer != nullptr)
		{
			if (Global::LocalPlayer->IsAlive())
			{
				if (b_IsThirdPerson && g_Settings->visuals_FakeAngleGhost != 0)
				{
					Vector OrigAng;
					OrigAng = ent->GetEyeAngles();

					if (g_Settings->visuals_FakeAngleGhost == 1)
						ent->SetAngle2(Vector(0, Global::FakeAngle.y, 0));
					else
						ent->SetAngle2(Vector(0, Global::LocalPlayer->LowerBodyYaw(), 0));

					g_pRenderView->SetColorModulation(g_Settings->color_FakeAngleGhost);
					g_pModelRender->ForcedMaterialOverride(fakeang_mat);
					ent->draw_model(0x1, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
					ent->SetAngle2(OrigAng);
				}
			}
		}

		if (ent->IsValidRenderable() && g_Settings->visuals_Chams)
		{
			if (g_Settings->visuals_ChamsWall)
			{
				g_pRenderView->SetColorModulation(g_Settings->color_ChamsWall);
				g_pModelRender->ForcedMaterialOverride(covered);
				ent->draw_model(0x1/*STUDIO_RENDER*/, 255);
				g_pModelRender->ForcedMaterialOverride(nullptr);
			}
			g_pRenderView->SetColorModulation(g_Settings->color_Chams);
			g_pModelRender->ForcedMaterialOverride(open);
			ent->draw_model(0x1/*STUDIO_RENDER*/, 255);
			g_pModelRender->ForcedMaterialOverride(nullptr);
		}

	}

	if (g_Settings->visuals_Glow || g_Settings->visuals_entGlow)
	{
		Glow::RenderGlow();
	}
}

void __fastcall Hooks::DrawModelExecute(void* ecx, void* edx, void* * ctx, void *state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
{
	if (!Global::LocalPlayer)
	{
		modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);
		return;
	}

	auto b_IsThirdPerson = *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(g_pInput) + 0xA5);

	if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		auto locplr = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());

		if (!locplr)
			return;

		if (pInfo.entity_index == g_pEngine->GetLocalPlayer() && locplr && b_IsThirdPerson && locplr->IsScoped())
			g_pRenderView->SetBlend(0.3f);
	}

	const char* ModelName = g_pModelInfo->GetModelName((model_t*)pInfo.pModel);

	modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);
	g_pModelRender->ForcedMaterialOverride(NULL);
}

IMaterial* __fastcall Hooks::FindMaterial(void* thisptr, void*, const char* pMaterialName, const char* pTextureGroupName, bool complain, const char* pComplainPrefix)
{
	static auto oFindMaterial = materialsystemVMT->GetOriginalMethod< FindMaterialFn >(84);

	if (strcmp(pMaterialName, "dev/scope_bluroverlay") == 0)
	{
		if (g_Settings->visuals_NoBlur)
		{
			static auto ClearBlur = oFindMaterial(thisptr, "dev/clearalpha", nullptr, complain, pComplainPrefix);
			return ClearBlur;
		}
	}

	return oFindMaterial(thisptr, pMaterialName, pTextureGroupName, complain, pComplainPrefix);
}