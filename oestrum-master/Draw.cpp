#include "sdk.h"
#include "Draw.h"
#include "xor.h"
#include "GameUtils.h"
#include "Math.h"

CDraw g_Draw;


void CDraw::Init()
{	
	font_esp = g_pSurface->SCreateFont();
	g_pSurface->SetFontGlyphSet(font_esp, XorStr("Tahoma"), 12, 700, 0, 0, FONTFLAG_OUTLINE);

	font_weapon = g_pSurface->SCreateFont();
	g_pSurface->SetFontGlyphSet(font_weapon, XorStr("undefeated"), 14, FW_NORMAL, 0, 0, FONTFLAG_ANTIALIAS);

	font_esp2 = g_pSurface->SCreateFont();
	g_pSurface->SetFontGlyphSet(font_esp2, XorStr("Smallest Pixel-7"), 11, 100, 0, 0, FONTFLAG_OUTLINE);
}


void CDraw::String(int x, int y, int r, int g, int b, const wchar_t *pszText)
{
	if (pszText == NULL)
		return;

	g_pSurface->DrawSetTextPos(x, y);
	g_pSurface->DrawSetTextFont(font_esp);
	g_pSurface->DrawSetTextColor(r, g, b, 255);
	g_pSurface->DrawPrintText(pszText, wcslen(pszText));
}

std::wstring CDraw::stringToWide(const std::string& text)
{
	std::wstring wide(text.length(), L' ');
	std::copy(text.begin(), text.end(), wide.begin());

	return wide;
}

int CDraw::getWidht(const char* input, unsigned int font)
{
	int iWide = 0;
	int iTall = 0;
	int iBufSize = MultiByteToWideChar(CP_UTF8, 0x0, input, -1, NULL, 0);

	wchar_t* pszUnicode = new wchar_t[iBufSize];

	MultiByteToWideChar(CP_UTF8, 0x0, input, -1, pszUnicode, iBufSize);

	g_pSurface->GetTextSize(font, pszUnicode, iWide, iTall);

	delete[] pszUnicode;

	return iWide;
}

void CDraw::DrawRect(int x1, int y1, int x2, int y2, Color clr)
{
    g_pSurface->SetDrawColor(clr);
	g_pSurface->DrawFilledRect(x1, y1, x2, y2);
}

void CDraw::StringA(unsigned long Font, bool center, int x, int y, int r, int g, int b, int a, const char *input, ...)
{
	CHAR szBuffer[MAX_PATH];

	if (!input)
		return;

	vsprintf_s(szBuffer, input, (char*)&input + _INTSIZEOF(input));


	if (center)
		x -= getWidht(szBuffer, Font) / 2;

	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextFont(Font);
	g_pSurface->DrawSetTextPos(x, y);

	std::wstring wide = stringToWide(std::string(szBuffer));
	g_pSurface->DrawPrintText(wide.c_str(), wide.length());
}

void CDraw::StringB(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int width, height;
	g_pSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void CDraw::FillRGBA(int x, int y, int w, int h, int r, int g, int b, int a)
{
	g_pSurface->SetDrawColor(r, g, b, a);
	g_pSurface->DrawFilledRect(x, y, x + w, y + h);
}
void CDraw::FillRGBAOutlined(int x, int y, int w, int h, int r, int g, int b, int a)
{
	FillRGBA(x, y, w, h, r, g, b, a);
	Box(x, y, w, h, 0, 0, 0, a);
}
void CDraw::Box(int x, int y, int w, int h, int r, int g, int b, int a)
{
	g_pSurface->SetDrawColor(r, g, b, a);
	g_pSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void CDraw::Draw3DCube(float scalar, QAngle angles, Vector middle_origin, Color outline)
{
	Vector forward, right, up;
	Math::AngleVectors(angles, forward, right, up);

	Vector points[8];
	points[0] = middle_origin - (right * scalar) + (up * scalar) - (forward * scalar); // BLT
	points[1] = middle_origin + (right * scalar) + (up * scalar) - (forward * scalar); // BRT
	points[2] = middle_origin - (right * scalar) - (up * scalar) - (forward * scalar); // BLB
	points[3] = middle_origin + (right * scalar) - (up * scalar) - (forward * scalar); // BRB

	points[4] = middle_origin - (right * scalar) + (up * scalar) + (forward * scalar); // FLT
	points[5] = middle_origin + (right * scalar) + (up * scalar) + (forward * scalar); // FRT
	points[6] = middle_origin - (right * scalar) - (up * scalar) + (forward * scalar); // FLB
	points[7] = middle_origin + (right * scalar) - (up * scalar) + (forward * scalar); // FRB

	Vector points_screen[8];
	for (int i = 0; i < 8; i++)
		if (!GameUtils::WorldToScreen(points[i], points_screen[i]))
			return;

	g_pSurface->SetDrawColor(outline);

	// Back frame
	g_pSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[1].x, points_screen[1].y);
	g_pSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[2].x, points_screen[2].y);
	g_pSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[1].x, points_screen[1].y);
	g_pSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[2].x, points_screen[2].y);

	// Frame connector
	g_pSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[4].x, points_screen[4].y);
	g_pSurface->DrawLine(points_screen[2].x, points_screen[2].y, points_screen[6].x, points_screen[6].y);
	g_pSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[7].x, points_screen[7].y);

	// Front frame
	g_pSurface->DrawLine(points_screen[4].x, points_screen[4].y, points_screen[5].x, points_screen[5].y);
	g_pSurface->DrawLine(points_screen[4].x, points_screen[4].y, points_screen[6].x, points_screen[6].y);
	g_pSurface->DrawLine(points_screen[7].x, points_screen[7].y, points_screen[5].x, points_screen[5].y);
	g_pSurface->DrawLine(points_screen[7].x, points_screen[7].y, points_screen[6].x, points_screen[6].y);
}

void CDraw::LineRGBA(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{

	g_pSurface->SetDrawColor(r, g, b, a);
	g_pSurface->DrawLine(x1, y1, x2, y2);
}
void TextW(bool cent, unsigned int font, int x, int y, int r, int g, int b, int a, wchar_t *pszString)
{
	if (cent)
	{
		int wide, tall;
		g_pSurface->GetTextSize(font, pszString, wide, tall);
		x -= wide / 2;
		y -= tall / 2;
	}
	g_pSurface->DrawSetTextColor(r, g, b, a);
	g_pSurface->DrawSetTextFont(font);
	g_pSurface->DrawSetTextPos(x, y);
	g_pSurface->DrawPrintText(pszString, (int)wcslen(pszString));
}

void CDraw::Text(int x, int y, int r, int g, int b, int a, bool bCenter, unsigned int font, const char *fmt, ...)
{
	va_list va_alist;
	char szBuffer[1024] = { '\0' };

	va_start(va_alist, fmt);
	vsprintf_s(szBuffer, fmt, va_alist);
	va_end(va_alist);

	wchar_t *pszStringWide = reinterpret_cast< wchar_t* >(malloc((strlen(szBuffer) + 1) * sizeof(wchar_t)));

	size_t outSize;
	size_t size = strlen(szBuffer) + 1;
	mbstowcs_s(&outSize, pszStringWide, size, szBuffer, size-1);

	TextW(bCenter, font, x, y, r, g, b, a, pszStringWide);

	free(pszStringWide);
}
void CDraw::ColoredCircle(int x, int y, int radius, int r, int g, int b, int a)
{
	//g_pSurface->DrawColoredCircle(x, y, radius, r, g, b, a);
	g_pSurface->SetDrawColor(r, g, b, a);
	g_pSurface->DrawOutlinedCircle(x, y, radius, 32);
}
void CDraw::DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
{
	Color colColor(0, 0, 0);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		FillRGBA(x + i, y, 1, height, colRainbow.r(), colRainbow.g(), colRainbow.b(), colRainbow.a());
	}
}
void CDraw::DrawTextureByteArray(Vector2D p0, Vector2D p1, Vector2D p2, Vector2D p3, const unsigned char* pRawRGBAData)
{
	static unsigned char buffer[4] = { 0,0,0,100 }; // R G B A (single pixel buffer layout.) 
	Vertex_t verts[4];
	static int Texture = g_pSurface->CreateNewTextureID(true);
	g_pSurface->DrawSetTextureRGBA(Texture, buffer/*pRawRGBAData*/, 1, 1);
	g_pSurface->SetDrawColor(255, 255, 255, 255);
	g_pSurface->DrawSetTexture(Texture);

	verts[0].Init(p0);
	verts[1].Init(p1);
	verts[2].Init(p2);
	verts[3].Init(p3);
	g_pSurface->DrawTexturedPolygon(4, verts);
}
