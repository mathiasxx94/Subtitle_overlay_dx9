#include "main.h"



// DX9
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;
ID3DXFont* pFontSmall = 0;
ID3DXFont* pFontBig = 0;
const MARGINS margin = { -1 };
LPD3DXSPRITE m_pSprite;


//WindowSize
int Width = GetSystemMetrics(SM_CXSCREEN);
int Height = GetSystemMetrics(SM_CYSCREEN);

const char* fonts[]{ "Meiryo", "MS PMincho", "MS Gothic", "IPA Gothic", "MS Mincho", "Sazanami Mincho", };
std::vector<timeSubpacket> subtitlePacket;

int currentfont{ 0 };
int fontHeight{ 70 };
float currenttime = 0;


int currentframe = 0;
bool ispaused = 1;
int yposoffset = 0;
bool showGui = 1;
bool enablebackground{ 1 };
bool enabledropshadow{ 1 };
bool enabletextborder{ 0 };

float textcolor[3]{ 1.f, 1.f, 1.f };
float backgroundcolor[3]{ 0.1f, 0.1f, 0.1f };
float dropshadowcolor[3]{ 0.01f, 0.01f, 0.01f };
float textbordercolor[3]{ 0.01f, 0.01f , 0.01f };



// Functions
int DrawingPart();
void fillWidths(ID3DXFont* ifont);    

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
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



int main()
{
	wchar_t hWindowName[256] = L"Subtitle_generator";


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
	
	MSG Message;
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

int D3D9XInit(HWND hWnd)
{
	IDirect3D9Ex* p_Object = 0;
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
	std::wstring  string;
	string = subtitlePacket[0].subline1;
	subtitlePacket[0].line1Width = calctextWidth(string.c_str(), pFontBig);

	string = subtitlePacket[0].subline2;
	subtitlePacket[0].line2Width = calctextWidth(string.c_str(), pFontBig);

	subtitlePacket[0].longestWidth = max(subtitlePacket[0].line1Width, subtitlePacket[0].line2Width);
	/*
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
	*/
}

int DrawStringW(const wchar_t* String, int x, int y, int r, int g, int b, int a, ID3DXFont* ifont)
{
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	ifont->DrawTextW(m_pSprite, String, wcslen(String), &FontPos, DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_ARGB(255, int(textcolor[0] * 255), int(textcolor[1] * 255), int(textcolor[2] * 255)));
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

void controlButtons()
{
	if (currenttime + 0.001 >= subtitlePacket[currentframe].secEnd)
	{
		if (currentframe!= subtitlePacket[subtitlePacket.size() -1].currframe)
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
	}
	if (GetAsyncKeyState(VK_LEFT) & 1)
	{
		currentframe -= 1;
		if (currentframe < 0) currentframe = 0;
		currenttime = subtitlePacket[currentframe].secStart;
	}
	if (GetAsyncKeyState(VK_SPACE) & 1)
	{
		ispaused = !ispaused;
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

/*
void assignFont2()
{
	if(pFontBig) 
	pFontBig->Release();
	size_t length = strlen(fonts[currentfont]);
	wchar_t text_wchar[30];
	mbstowcs(text_wchar, fonts[currentfont], length+1);
	D3DXCreateFontW(p_Device, fontHeight, 0, 0, 0, false, SHIFTJIS_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, text_wchar, &pFontBig); //MS PMincho 
}
*/


int DrawingPart()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (showGui)
	{
		ImGui::Begin("Terrace house subtitles");
		ImGui::SliderFloat("Timeline: ", &currenttime, 0.0f, subtitlePacket[subtitlePacket.size()-1].secStart);
		ImGui::SameLine();
		ImGui::Text("%0d min %0d sec", static_cast<int>(currenttime) / 60, static_cast<int>(currenttime) % 60);
		
		ImGuiCustom::colorPicker("Text color", textcolor); setvalidColor(textcolor);
		ImGuiCustom::colorPicker("Background", backgroundcolor, &enablebackground); setvalidColor(backgroundcolor);
		ImGuiCustom::colorPicker("Dropshadow", dropshadowcolor, &enabledropshadow); setvalidColor(dropshadowcolor);
		ImGuiCustom::colorPicker("Text border", textbordercolor, &enabletextborder); setvalidColor(textbordercolor);

		if (ImGui::InputInt("Font size", &fontHeight))
		{
			assignFont();
			fillWidths(pFontBig);
		}
		
		if (ImGui::Combo("Font", &currentfont, fonts, IM_ARRAYSIZE(fonts)))
		{
			assignFont();
			fillWidths(pFontBig);
		}
		
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