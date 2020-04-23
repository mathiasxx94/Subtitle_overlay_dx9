#pragma once
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

#include "imguicustom.h"
#include "utility.h"
#include "text parsing.h"

static float textcolor[3]{ 1.f, 1.f, 1.f };
static float backgroundcolor[3]{ 0.1f, 0.1f, 0.1f };
static float dropshadowcolor[3]{ 0.01f, 0.01f, 0.01f };
static float textbordercolor[3]{ 0.01f, 0.01f , 0.01f };

// Window
extern int Height, Width;
extern std::wstring ligne;
extern std::vector<timeSubpacket> subtitlePacket;

static float currenttime = 0;
static int currentframe = 0;
static bool ispaused = 1;
static int fontHeight = 70;
static int yposoffset = 0;
static bool showGui = 0;
static bool enablebackground{ 1 };
static bool enabledropshadow{ 1 };
static bool enabletextborder{ 0 };


static int currentfont{ 0 };
extern const char* fonts[];


// DX9
extern IDirect3D9Ex* p_Object;
extern IDirect3DDevice9Ex* p_Device;
extern D3DPRESENT_PARAMETERS p_Params;
extern ID3DXFont* pFontSmall;
extern ID3DXFont* pFontBig;
extern const MARGINS margin;
extern MSG Message;

/*
extern IDirect3D9Ex* p_Object = 0;
extern IDirect3DDevice9Ex* p_Device = 0;
extern D3DPRESENT_PARAMETERS p_Params;
extern ID3DXFont* pFontSmall = 0;
extern ID3DXFont* pFontBig = 0;
extern const MARGINS margin = { -1 };
extern MSG Message;
*/