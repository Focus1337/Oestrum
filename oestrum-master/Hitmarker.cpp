#include "Hitmarker.h"
#include "sdk.h"
#include "Draw.h"

void CHitmarker::update_end_time()
{
	this->end_time = g_pGlobals->curtime + 0.2f;
}
void CHitmarker::draw()
{
	if (g_pGlobals->curtime > this->end_time)
		return;
	if (g_Settings->visuals_Hitmarker)
	{
		static int lineSize = 5;

		static float alpha = 0;
		float step = 255.f / 0.3f * g_pGlobals->frametime;

		if (g_pGlobals->curtime + 0.4f >= g_pGlobals->curtime)
			alpha = 255.f;
		else
			alpha -= step;

		if (alpha > 0)
		{
			int screenSizeX, screenCenterX;
			int screenSizeY, screenCenterY;
			g_pEngine->GetScreenSize(screenSizeX, screenSizeY);

			screenCenterX = screenSizeX / 2;
			screenCenterY = screenSizeY / 2;

			g_Draw.LineRGBA(screenCenterX - lineSize * 2, screenCenterY - lineSize * 2, screenCenterX - (lineSize), screenCenterY - (lineSize), 230, 230, 230, alpha);
			g_Draw.LineRGBA(screenCenterX - lineSize * 2, screenCenterY + lineSize * 2, screenCenterX - (lineSize), screenCenterY + (lineSize), 230, 230, 230, alpha);
			g_Draw.LineRGBA(screenCenterX + lineSize * 2, screenCenterY + lineSize * 2, screenCenterX + (lineSize), screenCenterY + (lineSize), 230, 230, 230, alpha);
			g_Draw.LineRGBA(screenCenterX + lineSize * 2, screenCenterY - lineSize * 2, screenCenterX + (lineSize), screenCenterY - (lineSize), 230, 230, 230, alpha);
		}
	}
}
void CHitmarker::play_sound()
{
	if (g_Settings->visuals_Hitmarker)
		g_pSurface->PlaySoundS("buttons\\arena_switch_press_02.wav");
}
