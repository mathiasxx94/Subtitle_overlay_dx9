#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include "d3dx9.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")  

#include <windows.h>
#include <iostream>
#include <chrono>
#include <iomanip>

#include "text parsing.h"
#include "imguicustom.h"



// Window
wchar_t hWindowName[256] = L"Subtitle_generator"; 
int Height, Width;
std::wstring ligne;
std::vector<timeSubpacket> subtitlePacket;

static float currenttime = 0;
static int currentframe = 0;
static bool ispaused = 1;  
static int fontHeight = 70; //50
static int yposoffset = 0;
static bool showGui = 0;
static bool enablebackground{ 1 };
//static int fontsize;

static float textcolor[3]{ 1.f, 1.f, 1.f };
static float backgroundcolor[3]{ 0.1f, 0.1f, 0.1f };

// DX9
IDirect3D9Ex* p_Object = 0;
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;
ID3DXFont* pFontSmall = 0;
ID3DXFont* pFontBig = 0;
const MARGINS margin = { -1 };
MSG Message;

LPD3DXSPRITE m_pSprite;

// Functions
int DrawingPart();
void fillWidths(ID3DXFont* ifont);    

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//int DrawStringA(const char* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont);
int D3D9XInit(HWND hWnd);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

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
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8; // D3DFMT_X8R8G8B8
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		exit(1);
	}

	D3DXCreateFontA(p_Device, 20, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Calibri Light", &pFontSmall);
	D3DXCreateFontW(p_Device, fontHeight, 0, 0, 0, false, SHIFTJIS_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Meiryo", &pFontBig); //MS PMincho 
	D3DXCreateSprite(p_Device, &m_pSprite);

	//under test slett
	p_Device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

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

	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, hWindowName, hWindowName, WS_POPUP, 1, 1, Width, Height, 0, 0, 0, 0); //WS_EX_TRANSPARENT prevents detecting keypress on imgui
	SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	
	ShowWindowAsync(hWnd, 3);

	D3D9XInit(hWnd);

	//ImGui setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	//Init file load
	fillVector();
	fillWidths(pFontBig);
	

	for (;;)
	{
		auto t1 = std::chrono::high_resolution_clock::now();

		if (PeekMessage(&Message, hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&Message);
			TranslateMessage(&Message);
		}
		
		auto t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> fp_ms = t2 - t1; 
		if (!ispaused) currenttime += (fp_ms.count() / 1000.f); 
	}
	
	return 0;
}

int calctextWidth(const wchar_t* String, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = 2;
	FontPos.top = 2;
	ifont->DrawTextW(0, String, wcslen(String), &FontPos, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));
	int width = FontPos.right - FontPos.left;
	return width;
}

void fillWidths(ID3DXFont* ifont) 
{
	for (int n = 0; n<subtitlePacket.size(); n++)
	{
		std::wstring  string;
		switch (subtitlePacket[n].sublines)
		{
		case 1:
			string = subtitlePacket[n].subline1;
			subtitlePacket[n].line1Width = calctextWidth(string.c_str(), pFontBig);
			subtitlePacket[n].longestWidth = subtitlePacket[n].line1Width;

			break;
		case 2:
			string = subtitlePacket[n].subline1;
			subtitlePacket[n].line1Width = calctextWidth(string.c_str(), pFontBig);

			string = subtitlePacket[n].subline2;
			subtitlePacket[n].line2Width = calctextWidth(string.c_str(), pFontBig);

			subtitlePacket[n].longestWidth = max(subtitlePacket[n].line1Width, subtitlePacket[n].line2Width);

			break;

		case 3:
			string = subtitlePacket[n].subline1;
			subtitlePacket[n].line1Width = calctextWidth(string.c_str(), pFontBig);

			string = subtitlePacket[n].subline2;
			subtitlePacket[n].line2Width = calctextWidth(string.c_str(), pFontBig);

			string = subtitlePacket[n].subline3;
			subtitlePacket[n].line3Width = calctextWidth(string.c_str(), pFontBig);

			int var = max(subtitlePacket[n].line1Width, subtitlePacket[n].line2Width);

			subtitlePacket[n].longestWidth = max(var, subtitlePacket[n].line3Width);

			break;
		}
	}
}

int DrawStringW(const wchar_t* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	int red = textcolor[0] * 255;
	int green = textcolor[1] * 255;
	int blue = textcolor[2] * 255;
	ifont->DrawTextW(m_pSprite, String, wcslen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(255, red, green, blue));
	return 0;
}

int DrawStringW2(const wchar_t* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	ifont->DrawTextW(m_pSprite, String, wcslen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	return 0;
}

void DrawFilledRectangle(float x, float y, float w, float h, int a, int r, int g, int b)
{
	D3DCOLOR color = D3DCOLOR_ARGB(a, r, g, b);
	D3DRECT rect = { x, y, w, h };
	int red = backgroundcolor[0] * 255;
	int green = backgroundcolor[1] * 255;
	int blue = backgroundcolor[2] * 255;
	p_Device->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, red, green, blue), 0, 0);
}

void DrawFilledRect(int x0, int y0, int x1, int y1)
{
	int w = x1 - x0;
	int h = y1 - y0;
	ID3DXLine* g_pLine = 0;
	D3DXCreateLine(p_Device, &g_pLine);

	g_pLine->SetWidth(h);
	g_pLine->SetAntialias(0);

	D3DXVECTOR2 VertexList[2];
	VertexList[0].x = x0;
	VertexList[0].y = y0 + (h >> 1);
	VertexList[1].x = x0 + w;
	VertexList[1].y = y0 + (h >> 1);

	D3DXVECTOR2 lines[] = { D3DXVECTOR2(0.0f, 50.0f), D3DXVECTOR2(400.0f, 500.0f) };

	g_pLine->Begin();
	g_pLine->Draw(lines, 2, D3DCOLOR_ARGB(100, 10, 10, 10));
	g_pLine->End();
}

void controlButtons()
{
	if (currenttime + 0.001 >= subtitlePacket[currentframe].secEnd)
	{
		currentframe += 1;
	}
	if (currenttime + 0.001 <= subtitlePacket[currentframe].secStart)
	{
		if (currentframe != 0)
			currentframe -= 1;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 1)
	{
		currentframe += 1;
		currenttime = subtitlePacket[currentframe].secStart;

		system("CLS");
		std::cout << "Frame: " << currentframe << std::endl;
		std::cout << "Time: " << std::fixed << std::setprecision(2) << currenttime << " seconds";
	}
	if (GetAsyncKeyState(VK_LEFT) & 1)
	{
		currentframe -= 1;
		if (currentframe < 0) currentframe = 0;
		currenttime = subtitlePacket[currentframe].secStart;

		system("CLS");
		std::cout << "Frame: " << currentframe << std::endl;
		std::cout << "Time: " << std::fixed << std::setprecision(2) << currenttime << "seconds";
	}
	if (GetAsyncKeyState(VK_SPACE) & 1)
	{
		ispaused = !ispaused;

		system("CLS");
		std::cout << "Frame: " << currentframe << std::endl;
		std::cout << "Time: " << std::fixed << std::setprecision(2) << currenttime << "seconds";
	}
	if (GetAsyncKeyState(VK_UP) & 1)
	{
		yposoffset -= 2;
	}
	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		yposoffset += 2;
	}
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		showGui = !showGui;
	}
}

void setvalidColor(float param[3])
{
	if((param[0] <= 0.001f) && (param[1] <= 0.001f) && (param[2] <= 0.001f))
		std::fill(param, param + 3, 0.002f);
}

void reassignFont()
{
	pFontBig->Release();
	D3DXCreateFontW(p_Device, fontHeight, 0, 0, 0, false, SHIFTJIS_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Meiryo", &pFontBig); //MS PMincho 
}

int DrawingPart()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (showGui)
	{
		ImGui::Begin("gh");
		ImGui::SliderFloat("Timeline", &currenttime, 0.0f, 100.0f, "%.2fm");
		
		ImGuiCustom::colorPicker("Text color", textcolor);
		ImGuiCustom::colorPicker("Background color", backgroundcolor, &enablebackground);
		if (ImGui::InputInt("Font size", &fontHeight))
		{
			reassignFont();
			fillWidths(pFontBig);
		}

		setvalidColor(textcolor);
		setvalidColor(backgroundcolor);
		
		ImGui::End();
	}
	ImGui::EndFrame();

	p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	p_Device->BeginScene();
	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	
	controlButtons();

	
	if (currenttime+0.01 > subtitlePacket[currentframe].secStart && currenttime+0.01 < subtitlePacket[currentframe].secEnd)
	{
		switch (subtitlePacket[currentframe].sublines)
		{
		case 1:
			if (enablebackground)
			DrawFilledRectangle(Width / 2 - subtitlePacket[currentframe].longestWidth / 2 - 3, 1300 + yposoffset, Width / 2 + subtitlePacket[currentframe].longestWidth / 2 + 3, 1300 + 45 + yposoffset, 200, 10, 10, 10);
			DrawStringW(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2, 1300 + yposoffset, 255, 255, 255, 255, pFontBig);

			break;
		case 2:
			if(enablebackground)
			DrawFilledRectangle(Width / 2 - subtitlePacket[currentframe].longestWidth / 2 - 3, 1300 + yposoffset, Width / 2 + subtitlePacket[currentframe].longestWidth / 2 + 3, 1300 + 90 + yposoffset, 200, 10, 10, 10);
			DrawStringW2(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2 +2, 1300 + yposoffset, 10, 10, 10, 255, pFontBig); //dropshadow test
			DrawStringW2(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2 - 2, 1300 + yposoffset, 10, 10, 10, 255, pFontBig); //dropshadow test
			DrawStringW2(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2, 1300 + yposoffset+2, 10, 10, 10, 255, pFontBig); //dropshadow test
			DrawStringW2(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2, 1300 + yposoffset-2, 10, 10, 10, 255, pFontBig); //dropshadow test
			DrawStringW(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2, 1300 + yposoffset, 255, 0, 0, 255, pFontBig);
			DrawStringW(subtitlePacket[currentframe].subline2.c_str(), Width / 2 - subtitlePacket[currentframe].line2Width / 2, 1300 + 40 + yposoffset, 255, 255, 255, 255, pFontBig);

			break;

		case 3:
			if (enablebackground)
			DrawFilledRectangle(Width / 2 - subtitlePacket[currentframe].longestWidth / 2 - 3, 1300 + yposoffset, Width / 2 + subtitlePacket[currentframe].longestWidth / 2 + 3, 1300 + 135 + yposoffset, 200, 10, 10, 10);
			DrawStringW(subtitlePacket[currentframe].subline1.c_str(), Width / 2 - subtitlePacket[currentframe].line1Width / 2, 1300 + yposoffset, 255, 255, 255, 255, pFontBig);
			DrawStringW(subtitlePacket[currentframe].subline2.c_str(), Width / 2 - subtitlePacket[currentframe].line2Width / 2, 1300 + 40 + yposoffset, 255, 255, 255, 255, pFontBig);
			DrawStringW(subtitlePacket[currentframe].subline3.c_str(), Width / 2 - subtitlePacket[currentframe].line3Width / 2, 1300 + 80 + yposoffset, 255, 255, 255, 255, pFontBig);

			break;
		}
	}
	
	

	//ligne = subtitlePacket[currentframe].subline1;
	//DrawFilledRectangle(10, 55, 1000, 80, 200, 20, 20, 20);
	//std::wstring hei = std::to_wstring(currenttime);
	
	//DrawFilledRectangle(1, 1, 60, 10, 200, 20, 20, 20);
	//DrawStringW(subtitlePacket[currentframe].subline1.c_str(), Width/2 - subtitlePacket[currentframe].line1Width/2, 50, 255, 255, 255, 255, pFontBig); //(wchar_t*)L"（YOU(ユウ)）テラスハウスは 見ず知らずの男女６人が" instead of hei.c_str()
	//DrawStringW(hei.c_str(), 1, 1, 255, 255, 255, 255, pFontSmall); //delete this
	
	m_pSprite->End();
	p_Device->EndScene();
	p_Device->PresentEx(0, 0, 0, 0, 0);
	return 0;
}