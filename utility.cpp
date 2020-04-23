#include "main.h"

void setvalidColor(float param[3])
{
	if ((param[0] <= 0.001f) && (param[1] <= 0.001f) && (param[2] <= 0.001f))
		std::fill(param, param + 3, 0.005f);
}

void assignFont()
{
	if (pFontBig)
		pFontBig->Release();
	size_t length = strlen(fonts[currentfont]);
	wchar_t text_wchar[30];
	mbstowcs(text_wchar, fonts[currentfont], length + 1);
	D3DXCreateFontW(p_Device, fontHeight, 0, 0, 0, false, SHIFTJIS_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, text_wchar, &pFontBig); //MS PMincho 
}