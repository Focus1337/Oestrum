#include "sdk.h"
#include "ESP.h"
#include "global.h"
#include "xor.h"
#include "Math.h"
#include "Misc.h"
#include "GameUtils.h"
#include "Draw.h"
#include "Aimbot.h"
#include "Resolver.h"

CESP* g_ESP = new CESP;

char* GetWeaponName(CBaseCombatWeapon *pWeapon)
{
	int ID = pWeapon->WeaponID();

	switch (ID) {
	case 4:
		return "Glock";
	case 2:
		return "Elite";
	case 36:
		return "P250";
	case 30:
		return "Tec9";
	case 1:
		return "Deagle";
	case 32:
		return "P2000";
	case 3:
		return "FiveSeven";
	case 64:
		return "Revolver";
	case 63:
		return "CZ75 Auto";
	case 61:
		return "USP";
	case 35:
		return "Nova";
	case 25:
		return "XM1014";
	case 29:
		return "Sawed Off";
	case 14:
		return "M249";
	case 28:
		return "Negev";
	case 27:
		return "Mag7";
	case 17:
		return "Mac10";
	case 33:
		return "MP7";
	case 24:
		return "UMP-45";
	case 19:
		return "P90";
	case 26:
		return "Bizon";
	case 34:
		return "MP9";
	case 10:
		return "Famas";
	case 16:
		return "M4A4";
	case 40:
		return "SSG08";
	case 8:
		return "Aug";
	case 9:
		return "AWP";
	case 38:
		return "SCAR20";
	case 13:
		return "Galil";
	case 7:
		return "Ak47";
	case 39:
		return "SG553";
	case 11:
		return "G3SG1";
	case 60:
		return "M4A1-S";
	case 46:
	case 48:
		return "Molotov";
	case 44:
		return "Grenade";
	case 43:
		return "Flashbang";
	case 45:
		return "Smoke";
	case 47:
		return "Decoy";
	case 31:
		return "Taser";
	default:
		return "Knife";
	}
}

bool GetCBaseEntityBox(CBaseEntity* pBaseEntity, Vector& BotCenter, Vector& TopCenter, float& Left, float& Right, bool is_player)
{
	if (!pBaseEntity)
		return false;

	if (!is_player)
	{
		const VMatrix& trans = pBaseEntity->GetCollisionBoundTrans();

		CollisionProperty* collision = pBaseEntity->GetCollision();

		if (!collision)
			return false;

		Vector min, max;

		min = collision->VecMins();
		max = collision->VecMaxs();

		Vector points[] = { Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z) };

		auto vector_transform = [](const Vector in1, const VMatrix& in2)
		{
			auto dot_product = [](const Vector &v1, const float *v2)
			{
				float ret = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				return ret;
			};

			auto out = Vector();
			out[0] = dot_product(in1, in2[0]) + in2[0][3];
			out[1] = dot_product(in1, in2[1]) + in2[1][3];
			out[2] = dot_product(in1, in2[2]) + in2[2][3];
			return out;
		};

		Vector pointsTransformed[8];
		for (int i = 0; i < 8; i++)
		{
			pointsTransformed[i] = vector_transform(points[i], trans);
		}

		Vector pos = pBaseEntity->GetAbsOrigin();
		Vector flb;
		Vector brt;
		Vector blb;
		Vector frt;
		Vector frb;
		Vector brb;
		Vector blt;
		Vector flt;


		if (!GameUtils::WorldToScreen(pointsTransformed[3], flb) || !GameUtils::WorldToScreen(pointsTransformed[5], brt)
			|| !GameUtils::WorldToScreen(pointsTransformed[0], blb) || !GameUtils::WorldToScreen(pointsTransformed[4], frt)
			|| !GameUtils::WorldToScreen(pointsTransformed[2], frb) || !GameUtils::WorldToScreen(pointsTransformed[1], brb)
			|| !GameUtils::WorldToScreen(pointsTransformed[6], blt) || !GameUtils::WorldToScreen(pointsTransformed[7], flt))
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
		//+1 for each cuz of borders at the original box
		float left = flb.x;        // left
		float top = flb.y;        // top
		float right = flb.x;    // right
		float bottom = flb.y;    // bottom

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}
		BotCenter = Vector(right - ((right - left) / 2), top, 0);
		TopCenter = Vector(right - ((right - left) / 2), bottom, 0);
		Left = left;
		Right = right;
	}
	else
	{
		Vector org = pBaseEntity->GetAbsOrigin();
		Vector head;
		if ((*pBaseEntity->GetFlags() & FL_DUCKING))
			head = org + Vector(0.f, 0.f, 52.f);
		else
			head = org + Vector(0.f, 0.f, 72.f);

		Vector org_screen, head_screen;
		if (!GameUtils::WorldToScreen(org, org_screen) || !GameUtils::WorldToScreen(head, head_screen))
			return false;

		int height = int(org_screen.y - head_screen.y);
		int width = int(height / 2);
		Left = int(head_screen.x - width / 2);
		Right = int(head_screen.x + width / 2);
		BotCenter = Vector(head_screen.x, org_screen.y);
		TopCenter = BotCenter; TopCenter.y = head_screen.y;
	}
	return true;
}

void CESP::DrawBones(CBaseEntity* pBaseEntity)
{
	float Red = g_Settings->color_Skeletons[0] * 255;
	float Green = g_Settings->color_Skeletons[1] * 255;
	float Blue = g_Settings->color_Skeletons[2] * 255;

	studiohdr_t *studioHdr = g_pModelInfo->GetStudioModel(pBaseEntity->GetModel());
	if (studioHdr)
	{
		static matrix3x4_t boneToWorldOut[128];
		if (pBaseEntity->SetupBones2(boneToWorldOut, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_pGlobals->curtime))
		{
			for (int i = 0; i < studioHdr->numbones; i++)
			{
				mstudiobone_t *bone = studioHdr->pBone(i);
				if (!bone || !(bone->flags & BONE_USED_BY_HITBOX) || bone->parent == -1)
					continue;

				Vector bonePos1;
				if (!GameUtils::WorldToScreen(Vector(boneToWorldOut[i][0][3], boneToWorldOut[i][1][3], boneToWorldOut[i][2][3]), bonePos1))
					continue;

				Vector bonePos2;
				if (!GameUtils::WorldToScreen(Vector(boneToWorldOut[bone->parent][0][3], boneToWorldOut[bone->parent][1][3], boneToWorldOut[bone->parent][2][3]), bonePos2))
					continue;

				g_pSurface->SetDrawColor(Red, Green, Blue, g_Settings->visuals_Opacity);
				g_pSurface->DrawLine((int)bonePos1.x, (int)bonePos1.y, (int)bonePos2.x, (int)bonePos2.y);
			}
		}
	}
}

#pragma region Drawing stuff
RECT GetViewport()
{
	RECT Viewport = { 0, 0, 0, 0 };
	int w, h;
	g_pEngine->GetScreenSize(w, h);
	Viewport.right = w; Viewport.bottom = h;
	return Viewport;
}

void DrawTexturedPoly(int n, Vertex_t* vertice, Color col)
{
	static int texture_id = g_pSurface->CreateNewTextureID(true);
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	g_pSurface->DrawSetTextureRGBA(texture_id, buf, 1, 1);
	g_pSurface->SetDrawColor(col);
	g_pSurface->DrawSetTexture(texture_id);
	g_pSurface->DrawTexturedPolygon(n, vertice);
}

void DrawFilledCircle(Vector2D center, Color color, float radius, float points)
{
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + center.x, radius * sinf(a) + center.y)));

	DrawTexturedPoly((int)points, vertices.data(), color);
}

void DrawFilledCircle2(int x, int y, float points, float radius, Color color)
{
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

	DrawTexturedPoly((int)points, vertices.data(), color);
}

void DrawTriangle(Vector2D pos1, Vector2D pos2, Vector2D pos3, Color pColor)
{
	Vertex_t pVertex[3] = { pos1, pos2, pos3 };

	DrawTexturedPoly(3, pVertex, pColor);
}
#pragma endregion

void CESP::SpreadCrosshair()
{
	int width = 0;
	int height = 0;

	if (g_Settings->visuals_SpreadCircle)
	{
		g_pEngine->GetScreenSize(width, height);

		if (Global::LocalPlayer && Global::LocalPlayer->IsAlive())
		{
			CBaseCombatWeapon* pWeapon = Global::LocalPlayer->GetWeapon();
			float cone = pWeapon->GetSpread() + pWeapon->GetInaccuracy();

			RECT View = GetViewport();
			int MidX = View.right / 1.9999999999999;
			int MidY = View.bottom / 2;

			if (cone > 0.0f)
			{
				float Red, Green, Blue;
				Red = g_Settings->color_SpreadCircle[0] * 255;
				Green = g_Settings->color_SpreadCircle[1] * 255;
				Blue = g_Settings->color_SpreadCircle[2] * 255;

				if (cone < 0.01f) cone = 0.01f;
				float size = (cone * height) * 0.7f;
				Color color(Red, Green, Blue, 10);

				g_pSurface->SetDrawColor(color);
				DrawFilledCircle(Vector2D(MidX, MidY), color, (int)size, 70);	
			}
		}
	}
}

#pragma region Antiaim angles draw
void CESP::DrawAngleLines()
{
	Vector src3d, dst3d, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = Global::LocalPlayer;

	if (g_Settings->visuals_ShowLby)
	{
		Math::AngleVectors(QAngle(0, Global::LocalPlayer->LowerBodyYaw(), 0), &forward);
		src3d = Global::LocalPlayer->GetOrigin();
		dst3d = src3d + (forward * 40.f);

		ray.Init(src3d, dst3d);

		g_pEngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!GameUtils::WorldToScreen(src3d, src) || !GameUtils::WorldToScreen(tr.endpos, dst))
			return;

		g_pSurface->SetDrawColor(Color(255, 165, 0, 255));
		g_pSurface->DrawLine(src.x, src.y, dst.x, dst.y);
		g_Draw.StringA(g_Draw.font_esp, false, dst.x, dst.y, 255, 165, 0, 255, "lby");
	}

	if (g_Settings->visuals_ShowReal)
	{
		Math::AngleVectors(QAngle(0, Global::RealAngle.y, 0), &forward);
		src3d = Global::LocalPlayer->GetOrigin();
		dst3d = src3d + (forward * 40.f);

		ray.Init(src3d, dst3d);

		g_pEngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!GameUtils::WorldToScreen(src3d, src) || !GameUtils::WorldToScreen(tr.endpos, dst))
			return;

		g_pSurface->SetDrawColor(Color(0, 255, 0, 255));
		g_pSurface->DrawLine(src.x, src.y, dst.x, dst.y);
		g_Draw.StringA(g_Draw.font_esp, false, dst.x, dst.y, 0, 255, 0, 255, "real");
	}

	if (g_Settings->visuals_ShowFake)
	{
		Math::AngleVectors(QAngle(0, Global::FakeAngle.y, 0), &forward);
		src3d = Global::LocalPlayer->GetOrigin();
		dst3d = src3d + (forward * 40.f);

		ray.Init(src3d, dst3d);

		g_pEngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!GameUtils::WorldToScreen(src3d, src) || !GameUtils::WorldToScreen(tr.endpos, dst))
			return;

		g_pSurface->SetDrawColor(Color(255, 0, 0, 255));
		g_pSurface->DrawLine(src.x, src.y, dst.x, dst.y);
		g_Draw.StringA(g_Draw.font_esp, false, dst.x, dst.y, 255, 0, 0, 255, "fake");
	}
}

static float degrees_to_radians(float deg)
{
	return deg * (M_PI / 180.f);
}

Vector get_rotated_position(Vector start, const float rotation, const float distance)
{
	const auto rad = degrees_to_radians(rotation);
	start.x += cosf(rad) * distance;
	start.y += sinf(rad) * distance;

	return start;
};

void CESP::DrawAngleArrows()
{
	auto pLocal = static_cast<CBaseEntity*>(g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer()));
	if (!pLocal)
		return;

	static const auto real_color = Color(0, 255, 0, 255);
	static const auto fake_color = Color(255, 0, 0, 255);
	static const auto lby_color = Color(255, 165, 0, 255);

	auto client_viewangles = Vector();
	auto screen_width = 0, screen_height = 0;

	g_pEngine->GetViewAngles(client_viewangles);
	g_pEngine->GetScreenSize(screen_width, screen_height);

	constexpr auto radius = 80.f;

	const auto screen_center = Vector2D(screen_width / 2.f, screen_height / 2.f);
	const auto real_rot = degrees_to_radians(client_viewangles.y - Global::RealAngle.y - 90);
	const auto fake_rot = degrees_to_radians(client_viewangles.y - Global::FakeAngle.y - 90);
	const auto lby_rot = degrees_to_radians(client_viewangles.y - pLocal->LowerBodyYaw() - 90);

	auto draw_arrow = [&](float rot, Color color) -> void
	{
		DrawTriangle(
			Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius),
			Vector2D(screen_center.x + cosf(rot + degrees_to_radians(10)) * (radius - 25.f),
				screen_center.y + sinf(rot + degrees_to_radians(10)) * (radius - 25.f)),
			Vector2D(screen_center.x + cosf(rot - degrees_to_radians(10)) * (radius - 25.f),
				screen_center.y + sinf(rot - degrees_to_radians(10)) * (radius - 25.f)),
			color);
	};

	if (g_Settings->visuals_ShowReal)
		draw_arrow(real_rot, real_color);
	if (g_Settings->visuals_ShowFake)
		draw_arrow(fake_rot, fake_color);
	if (g_Settings->visuals_ShowLby)
		draw_arrow(lby_rot, lby_color);
}
#pragma endregion

void CESP::DrawPlayer(CBaseEntity* pPlayer, CBaseEntity* pLocalPlayer)
{
	Vector m_position = pPlayer->GetAbsOrigin();
	Vector m_top_position = m_position;

	float pAlpha = g_Settings->visuals_Opacity;

	if (*pPlayer->GetFlags() & IN_DUCK)
		m_top_position += Vector(0, 0, 52);
	else
		m_top_position += Vector(0, 0, 72);

	Vector footpos, headpos;

	if (GameUtils::WorldToScreen(m_position, footpos) && GameUtils::WorldToScreen(m_top_position, headpos))
	{
		Vector Bot, Top;
		float Left, Right;
		GetCBaseEntityBox(pPlayer, Bot, Top, Left, Right, true);
		int height = Bot.y - Top.y;
		int width = Right - Left;
		int x = Left;
		int y = Top.y;
		auto place = 0;

		CBaseCombatWeapon* pMainWeapon = pPlayer->GetWeapon();

		if (g_Settings->visuals_Offscreen)
		{
			float Red = g_Settings->color_OffScreen[0] * 255;
			float Green = g_Settings->color_OffScreen[1] * 255;
			float Blue = g_Settings->color_OffScreen[2] * 255;

			Vector eyeangles, focusdaun;
			int screen_w, screen_h;
			g_pEngine->GetScreenSize(screen_w, screen_h);
			g_pEngine->GetViewAngles(eyeangles);
			QAngle newangle = Math::CalcAngle(Vector(Global::LocalPlayer->GetAbsOrigin().x, Global::LocalPlayer->GetAbsOrigin().y, 0), Vector(pPlayer->GetAbsOrigin().x, pPlayer->GetAbsOrigin().y, 0));
			Math::AngleVectors(QAngle(0, 270, 0) - newangle + QAngle(0, eyeangles.y, 0), &focusdaun);
			auto circlevec = Vector(screen_w / 2, screen_h / 2, 0) + (focusdaun * 250.f);
			if (pPlayer != Global::LocalPlayer && pPlayer->IsAlive() && Global::LocalPlayer->IsAlive() && !Global::LocalPlayer->IsDormant()
				&& Global::LocalPlayer->GetHealth() > 0)
			DrawFilledCircle2(circlevec.x, circlevec.y, 20, 20, Color(Red, Green, Blue, 125));
		}

		if (g_Settings->visuals_Bones)
			DrawBones(pPlayer);

		if (g_Settings->visuals_BoundingBox)
		{
			float Red = g_Settings->color_BoundingBox[0] * 255;
			float Green = g_Settings->color_BoundingBox[1] * 255;
			float Blue = g_Settings->color_BoundingBox[2] * 255;

			g_Draw.Box(x - 1, y - 1, width + 2, height + 2, 0, 0, 0, pAlpha - 30);
			g_Draw.Box(x, y, width, height, Red, Green, Blue, pAlpha);
			g_Draw.Box(x + 1, y + 1, width - 2, height - 2, 0, 0, 0, pAlpha - 30);
		}

		if (g_Settings->visuals_ResolverInfo)
		{
			float Red, Green, Blue;
			Red = g_Settings->color_ResolverInfo[0] * 255;
			Green = g_Settings->color_ResolverInfo[1] * 255;
			Blue = g_Settings->color_ResolverInfo[2] * 255;

			if (g_Settings->rage_Resolver)
			{
				auto idx = pPlayer->GetIndex();
				if (pPlayer->IsAlive() && pPlayer->GetHealth() > 0 && !pPlayer->IsDormant()
					&& Global::LocalPlayer->IsAlive() && Global::LocalPlayer->GetHealth() > 0 && !Global::LocalPlayer->IsDormant())
				{
					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), Red, Green, Blue, pAlpha, "%s", g_PlayerSettings[idx].resolveType);
				}
			}
		}

		/*if (g_Settings->test_Animtest && g_Settings->rage_Resolver)
		{
			for (int s = 0; s < g_Settings->test_Animcount; s++)
			{
				AnimationLayer layer = pPlayer->GetAnimOverlay(s);
				const int activity = pPlayer->GetSequenceActivity(layer.m_nSequence);
				float flcycle = layer.m_flCycle, flprevcycle = layer.m_flPrevCycle, flweight = layer.m_flWeight;

				if (pPlayer->IsAlive() && pPlayer->GetHealth() > 0 && !pPlayer->IsDormant())
				{
					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), 255, 255, 255, pAlpha, "ACT: %i", activity);
					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), 255, 255, 255, pAlpha, "CYCLE: %f", flcycle);
					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), 255, 255, 255, pAlpha, "PREV CYCLE: %f", flprevcycle);
					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), 255, 255, 255, pAlpha, "WEIGHT: %f", flweight);
				}
			}
		}*/

		if (g_Settings->visuals_BotInfo)
		{
			float Red, Green, Blue;
			Red = g_Settings->color_BotInfo[0] * 255;
			Green = g_Settings->color_BotInfo[1] * 255;
			Blue = g_Settings->color_BotInfo[2] * 255;

			if (pPlayer->IsAlive() && pPlayer->GetHealth() > 0 && !pPlayer->IsDormant())
			{
				auto idx = pPlayer->GetIndex();
				player_info_t plInfo;

				if (pPlayer && g_pEngine->GetPlayerInfo(idx, &plInfo))
				{
					if (plInfo.m_bIsFakePlayer)
						g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), Red, Green, Blue, pAlpha, "BOT");
				}
			}
		}

		if (g_Settings->visuals_Health)
		{
			int health = 100 - pPlayer->GetHealth();
			int w = 4;
			if (width < 4)
				w = width;

			int hr, hg, hb;

			hr = 255 - (pPlayer->GetHealth()*2.55);
			hg = pPlayer->GetHealth() * 2.55;
			hb = 0;

			g_Draw.FillRGBA(x - (5), y, w, height, 0, 0, 0, pAlpha - 65);
			g_Draw.FillRGBA(x - (5), y + health * height / 100, w, pPlayer->GetHealth()*height / 100, hr, hg, hb, pAlpha);
			g_Draw.Box(x - (5), y, w, height, 0, 0, 0, pAlpha - 20);

			char hp[20];
			sprintf_s(hp, sizeof(hp), "%i", pPlayer->GetHealth());

			g_Draw.StringA(g_Draw.font_esp2, false, x - g_Draw.getWidht(hp, g_Draw.font_esp2), y + health * height / 100 - 6, 255, 255, 255, pAlpha, "%i", pPlayer->GetHealth());
		}


		int bot_add = 0;
		int top_add = 0;

		if (g_Settings->visuals_Name)
		{
			player_info_s info;
			g_pEngine->GetPlayerInfo(pPlayer->GetIndex(), &info);
			g_Draw.StringA(g_Draw.font_esp, true, x + width / 2, y - 12 - top_add, 255, 255, 255, pAlpha, "%s", info.m_szPlayerName);
			top_add += 12;
		}

		UINT* hWeapons = pPlayer->GetWeapons();
		if (hWeapons)
		{
			if (pMainWeapon)
			{
				if (g_Settings->visuals_Ammo && pMainWeapon && pMainWeapon->Clip1() != -1 && pMainWeapon->GetWeaponType() != WEAPONCLASS::WEPCLASS_INVALID && !pMainWeapon->IsGrenade() && !pMainWeapon->IsKnife())
				{
					std::string ammo_str;

					ammo_str.append(to_string(pMainWeapon->Clip1()));
					ammo_str.append(XorStr("/"));
					ammo_str.append(to_string(pMainWeapon->GetCSWpnData()->iMaxClip1));

					g_Draw.StringA(g_Draw.font_esp2, false, x + width + 3, y + (place++ * 10), 255, 255, 255, pAlpha, XorStr("%s"), ammo_str.c_str());
				}

				if (g_Settings->visuals_Weapon != 0)
				{
					if (g_Settings->visuals_Weapon == 1)
					{
						g_Draw.StringA(g_Draw.font_esp2, true, x + width / 2, y + height + bot_add + 5, 255, 255, 255, pAlpha, XorStr("%s"), GetWeaponName(pMainWeapon));
					}
					else if (g_Settings->visuals_Weapon == 2)
					{
						g_Draw.StringA(g_Draw.font_weapon, true, x + width / 2, y + height + bot_add + 5, 255, 255, 255, pAlpha, XorStr("%s"), pMainWeapon->GetGunIcon());
					}
				}
			}
		}

		if (g_Settings->visuals_Ammo)
		{
			auto animLayer = pPlayer->GetAnimOverlay(1);

			if (!animLayer.m_pOwner)
				return;

			if (pMainWeapon)
			{
				int iClip = pMainWeapon->Clip1();
				int iClipMax = pMainWeapon->GetCSWpnData()->iMaxClip1;
				auto activity = pPlayer->GetSequenceActivity(animLayer.m_nSequence);

				Vector Bot, Top;
				float Left, Right;
				GetCBaseEntityBox(pPlayer, Bot, Top, Left, Right, true);
				int height = Bot.y - Top.y;
				float box_w = Right - Left;
				int x = Left;
				int y = Top.y;

				float width;
				float width2 = (((box_w * iClip) / iClipMax));

				float Red, Green, Blue;
				Red = g_Settings->color_Ammobar[0] * 255;
				Green = g_Settings->color_Ammobar[1] * 255;
				Blue = g_Settings->color_Ammobar[2] * 255;

				if (activity == 967 && animLayer.m_flWeight != 0.f)
				{
					float cycle = animLayer.m_flCycle;
					width = (((box_w * cycle) / 1.f));
				}
				else
					width = (((box_w * iClip) / iClipMax));

				if (pMainWeapon && pMainWeapon->Clip1() != -1 && pMainWeapon->GetWeaponType() != WEAPONCLASS::WEPCLASS_INVALID && !pMainWeapon->IsGrenade() && !pMainWeapon->IsKnife())
				{
					g_Draw.DrawRect(Left - 1, Bot.y + 1, Right + 1, Bot.y + 6, Color(0, 0, 0, pAlpha - 60));
					g_Draw.DrawRect(Left, Bot.y + 2, Left + width, Bot.y + 5, Color(Red, Green, Blue));
					bot_add = 7;
				}
			}
		}
	}
}

void CESP::Run()
{
	if (!Global::LocalPlayer)
		return;

	if (Global::LocalPlayer->IsAlive())
		SpreadCrosshair();

	auto b_IsThirdPerson = *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(g_pInput) + 0xA5);

	if (Global::LocalPlayer->IsAlive() && (g_Settings->visuals_ShowReal || g_Settings->visuals_ShowFake || g_Settings->visuals_ShowLby))
	{
		if (b_IsThirdPerson)
			DrawAngleLines();
		else
			DrawAngleArrows();
	}

	for (int i = 1; i <= g_pEntitylist->GetHighestEntityIndex(); i++)
	{
		CBaseEntity* pBaseEntity = g_pEntitylist->GetClientEntity(i);

		if (!pBaseEntity)
			continue;

		ClientClass* pClass = (ClientClass*)pBaseEntity->GetClientClass();

		if (pClass->m_ClassID == int(ClassId_CCSPlayer) && g_Settings->visuals_Enabled)
		{
			if (pBaseEntity->IsDormant())
				continue;

			if (pBaseEntity->GetHealth() <= 0)
				continue;

			if (!pBaseEntity->IsAlive())
				continue;

			bool is_friendly = pBaseEntity->GetTeamNum() == Global::LocalPlayer->GetTeamNum();

			if (!is_friendly || is_friendly && !g_Settings->visuals_EnemyOnly)
				g_ESP->DrawPlayer(pBaseEntity, Global::LocalPlayer);
		}

		Vector buf, pos = pBaseEntity->GetAbsOrigin();

		if (pos.x == 0 || pos.y == 0 || pos.z == 0 || !GameUtils::WorldToScreen(pos, buf))
			continue;

		if ((pClass->m_ClassID != int(ClassId_CBaseWeaponWorldModel) && (strstr(pClass->m_pNetworkName, XorStr("Weapon")) || pClass->m_ClassID == int(ClassId_CDEagle) || pClass->m_ClassID == int(ClassId_CAK47))) && g_Settings->visuals_DroppedWeapon)
			DrawDroppedWeapon((CBaseCombatWeapon*)pBaseEntity);

		if (strstr(pClass->m_pNetworkName, XorStr("Projectile")) && g_Settings->visuals_Nades)
			DrawNades(pBaseEntity);

		if ((pClass->m_ClassID == int(ClassId_CC4) || pClass->m_ClassID == int(ClassId_CPlantedC4)))
		{
			if (g_Settings->visuals_PlantedBomb)
				DrawPlantedBomb(pBaseEntity, pClass->m_ClassID == int(ClassId_CPlantedC4), Global::LocalPlayer);
		}
	}
}

void CESP::DrawScope(CBaseEntity* pLocalPlayer)
{
	CBaseCombatWeapon* pLocalWeapon = pLocalPlayer->GetWeapon();
	auto b_IsThirdPerson = *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(g_pInput) + 0xA5);

	if (pLocalWeapon)
	{
		if (pLocalWeapon->IsSniper() || (pLocalWeapon->IsScopeAuto() && b_IsThirdPerson))
		{
			if (pLocalPlayer->IsScoped())
			{
				int width = 0;
				int height = 0;
				g_pEngine->GetScreenSize(width, height);
				int centerX = static_cast<int>(width * 0.5f);
				int centerY = static_cast<int>(height * 0.5f);
				g_pSurface->SetDrawColor(Color(0, 0, 0, 255));
				g_pSurface->DrawLine(0, centerY, width, centerY);
				g_pSurface->DrawLine(centerX, 0, centerX, height);
			}
		}
	}
}

void CESP::DrawPlantedBomb(CBaseEntity* pBomb, bool is_planted, CBaseEntity* pLocalPlayer)
{
	Vector Bot, Top;
	float Left, Right;
	GetCBaseEntityBox(pBomb, Bot, Top, Left, Right, false);
	int height = Bot.y - Top.y;
	int width = Right - Left;

	float boxRed = g_Settings->color_entBox[0] * 255;
	float boxGreen = g_Settings->color_entBox[1] * 255;
	float boxBlue = g_Settings->color_entBox[2] * 255;

	if (g_Settings->visuals_entBox)
	{
		g_Draw.Box(Left, Top.y, width, height, 0, 0, 0, 255);
		g_Draw.Box(Left + 1, Top.y + 1, width - 2, height - 2, boxRed, boxGreen, boxBlue, 255);
		g_Draw.Box(Left + 2, Top.y + 2, width - 4, height - 4, 0, 0, 0, 255);
	}
	if (g_Settings->visuals_entName)
	{
		g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y, 200, 200, 200, 255, "%s", "Bomb");

		if (is_planted)
		{
			CBomb* bomb = (CBomb*)pBomb;
			float flBlow = bomb->GetC4BlowTime();
			float TimeRemaining = flBlow - (g_pGlobals->interval_per_tick * pLocalPlayer->GetTickBase());
			if (TimeRemaining < 0)
				TimeRemaining = 0;
			char buffer[64];

			if (bomb->IsDefused())
				sprintf_s(buffer, XorStr("Defused"));
			else
				sprintf_s(buffer, XorStr("B: %.1fs"), TimeRemaining);

			g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y + 13, 200, 200, 200, 255, "%s", buffer);

			if (!bomb->IsDefused())
			{
				float a = 450.7f;
				float b = 75.68f;
				float c = 789.2f;
				float d = ((pLocalPlayer->GetOrigin().DistTo(pBomb->GetOrigin()) - b) / c);
				float flDamage = a * expf(-d * d);

				auto GetArmourHealth = [](float flDamage, int ArmorValue)
				{
					float flCurDamage = flDamage;

					if (flCurDamage == 0.0f || ArmorValue == 0)
						return flCurDamage;

					float flArmorRatio = 0.5f;
					float flArmorBonus = 0.5f;
					float flNew = flCurDamage * flArmorRatio;
					float flArmor = (flCurDamage - flNew) * flArmorBonus;

					if (flArmor > ArmorValue)
					{
						flArmor = ArmorValue * (1.0f / flArmorBonus);
						flNew = flCurDamage - flArmor;
					}

					return flNew;
				};

				float damage = max((int)ceilf(GetArmourHealth(flDamage, pLocalPlayer->GetArmor())), 0);

				g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y + 26, 200, 200, 200, 255, "D: %f", damage);
			}
		}
	}
}

void CESP::DrawNades(CBaseEntity* pThrowable)
{
	const model_t* nadeModel = pThrowable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = g_pModelInfo->GetStudioModel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->name, XorStr("thrown")) && !strstr(hdr->name, XorStr("dropped")))
		return;

	std::string nadeName = XorStr("Unknown Grenade");

	IMaterial* mats[32];
	g_pModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;

		if (strstr(mat->GetName(), XorStr("flashbang")))
		{
			nadeName = XorStr("Flashbang");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("m67_grenade")) || strstr(mat->GetName(), XorStr("hegrenade")))
		{
			nadeName = XorStr("Grenade");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("smoke")))
		{
			nadeName = XorStr("Smoke");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("decoy")))
		{
			nadeName = XorStr("Decoy");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("incendiary")) || strstr(mat->GetName(), XorStr("molotov")))
		{
			nadeName = XorStr("Molotov");

			break;
		}
	}
	Vector Bot, Top;
	float Left, Right;
	GetCBaseEntityBox(pThrowable, Bot, Top, Left, Right, false);
	int height = Bot.y - Top.y;
	int width = Right - Left;

	float boxRed = g_Settings->color_entBox[0] * 255;
	float boxGreen = g_Settings->color_entBox[1] * 255;
	float boxBlue = g_Settings->color_entBox[2] * 255;

	float NameRed = g_Settings->color_entName[0] * 255;
	float NameGreen = g_Settings->color_entName[1] * 255;
	float NameBlue = g_Settings->color_entName[2] * 255;

	if (g_Settings->visuals_entBox)
	{
		g_Draw.Box(Left, Top.y, width, height, 0, 0, 0, 255);
		g_Draw.Box(Left + 1, Top.y + 1, width - 2, height - 2, boxRed, boxGreen, boxBlue, 255);
		g_Draw.Box(Left + 2, Top.y + 2, width - 4, height - 4, 0, 0, 0, 255);
	}
	if (g_Settings->visuals_entName)
	{
		g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y, NameRed, NameGreen, NameBlue, 255, nadeName.c_str());
	}
}

void CESP::DrawDroppedWeapon(CBaseCombatWeapon* pWeapon)
{
	CBaseEntity* pCBaseEntity = (CBaseEntity*)pWeapon;
	CBaseCombatWeapon* Weapon = (CBaseCombatWeapon*)pWeapon;

	if (!pCBaseEntity || !g_pModelInfo)
		return;

	std::string sCurWeapon = g_pModelInfo->GetModelName(pCBaseEntity->GetModel());
	if (!sCurWeapon.empty() && sCurWeapon.find(XorStr("w_")) != std::string::npos)
	{
		std::string name;

		if (sCurWeapon.find(XorStr("defuser")) != std::string::npos/* insert check for defuser here*/)
			name = XorStr("defuser");
		else
		{
			CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pCBaseEntity;
			if (!pWeapon)return;
			name = GetWeaponName(pWeapon);

			if (pWeapon && pWeapon->Clip1() != -1 && pWeapon->GetWeaponType() != WEAPONCLASS::WEPCLASS_INVALID && !pWeapon->IsGrenade() && !pWeapon->IsKnife())
			{
				name.append(XorStr(" ("));
				name.append(to_string(pWeapon->Clip1()));
				name.append(XorStr("/"));
				name.append(to_string(pWeapon->GetCSWpnData()->iMaxClip1));
				name.append(XorStr(")"));
			}
		}

		Vector Bot, Top;
		float Left, Right;
		GetCBaseEntityBox(pCBaseEntity, Bot, Top, Left, Right, false);
		int height = Bot.y - Top.y;
		int width = Right - Left;

		float boxRed = g_Settings->color_entBox[0] * 255;
		float boxGreen = g_Settings->color_entBox[1] * 255;
		float boxBlue = g_Settings->color_entBox[2] * 255;

		float NameRed = g_Settings->color_entName[0] * 255;
		float NameGreen = g_Settings->color_entName[1] * 255;
		float NameBlue = g_Settings->color_entName[2] * 255;

		if (g_Settings->visuals_entBox)
		{
			g_Draw.Box(Left, Top.y, width, height, 0, 0, 0, 255);
			g_Draw.Box(Left + 1, Top.y + 1, width - 2, height - 2, boxRed, boxGreen, boxBlue, 255);
			g_Draw.Box(Left + 2, Top.y + 2, width - 4, height - 4, 0, 0, 0, 255);
		}
		if (g_Settings->visuals_entName)
		{
			g_Draw.StringA(g_Draw.font_esp2, true, Left + width / 2, Bot.y, NameRed, NameGreen, NameBlue, 255, name.c_str());
		}
	}

}

