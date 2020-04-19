#include <d3d9.h>
#include "d3dx9.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <windows.h>
#include <iostream>

#include "text parsing.h"



// Window
wchar_t hWindowName[256] = L"Subtitle_generator";
int Height, Width;
std::wstring ligne;
std::vector<timeSubpacket> subtitlePacket;

// DX9
IDirect3D9Ex* p_Object = 0;
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;
ID3DXFont* pFontSmall = 0;
ID3DXFont* pFontBig = 0;
const MARGINS margin = { -1 };
MSG Message;
int r = 1;


// Functions
int DrawingPart();
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//int DrawStringA(const char* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont);
int D3D9XInit(HWND hWnd);

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_PAINT:
		DrawingPart();
		break;

	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &margin);
		break;

	case WM_DESTROY:
		PostQuitMessage(1);
		return 0;

	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

int D3D9XInit(HWND hWnd)
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
	{
		exit(1);
	}

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_X8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		exit(1);
	}

	D3DXCreateFontA(p_Device, 12, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Calibri Light", &pFontSmall);
	D3DXCreateFontW(p_Device, 38, 0, 0, 0, false, SHIFTJIS_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS PMincho", &pFontBig); //Meiryo

	return 0;
}

int main()
{
	Width = GetSystemMetrics(SM_CXSCREEN);
	Height = GetSystemMetrics(SM_CYSCREEN);

	// DX9
	WNDCLASSEX wClass;
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wClass.hInstance = GetModuleHandle(0);
	wClass.lpfnWndProc = WinProc;
	wClass.lpszClassName = hWindowName;
	wClass.lpszMenuName = hWindowName;
	wClass.style = CS_VREDRAW | CS_HREDRAW;
	if (!RegisterClassEx(&wClass)) { exit(1); }

	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, hWindowName, hWindowName, WS_POPUP, 1, 1, Width, Height, 0, 0, 0, 0);
	SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	ShowWindowAsync(hWnd, 3);

	D3D9XInit(hWnd);

	//Init file load
	
	fillVector();
	


	for (;;)
	{
		if (PeekMessage(&Message, hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&Message);
			TranslateMessage(&Message);
		}
		Sleep(10);
	}
	
	return 0;
}


int DrawStringA(char* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	ifont->DrawTextA(0, String, strlen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(a, r, g, b));
	return 0;
}

int DrawStringW(const wchar_t* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	//ifont->DrawTextA(0, String, strlen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(a, r, g, b));
	ifont->DrawTextW(0, String, wcslen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(a, r, g, b));
	return 0;
}

void DrawFilledRectangle(float x, float y, float w, float h, int a, int r, int g, int b)
{
	D3DCOLOR color = D3DCOLOR_ARGB(a, r, g, b);
	D3DRECT rect = { x, y, w, h };
	p_Device->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, color, 0, 0);
}

int DrawingPart()
{
	p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	p_Device->BeginScene();


	r++;
	if (r >= 255) { r = 1; }
	
	ligne = subtitlePacket[r].subline2;
	DrawFilledRectangle(10, 55 + r, 1000, 80 + r, 200, 20, 20, 20);
	//DrawStringA((char*)"hallo", 10, 50, r, g, b, 150, pFontBig);
	//std::wstring hei{L"テラスハウスは" };

	DrawStringW(ligne.c_str(), 10, 50 + r, 255, 255, 255, 255, pFontBig); //(wchar_t*)L"（YOU(ユウ)）テラスハウスは 見ず知らずの男女６人が" instead of hei.c_str()


	p_Device->EndScene();
	p_Device->PresentEx(0, 0, 0, 0, 0);
	return 0;
}