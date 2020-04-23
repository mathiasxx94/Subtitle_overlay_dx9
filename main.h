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

// Window
extern int Height, Width;

//Globals
extern std::vector<timeSubpacket> subtitlePacket;
extern float currenttime;
extern int currentframe;
extern bool ispaused;
extern int yposoffset;
extern bool showGui;

extern bool enablebackground;
extern bool enabledropshadow;
extern bool enabletextborder;

extern float textcolor[];
extern float backgroundcolor[];
extern float dropshadowcolor[];
extern float textbordercolor[];

extern int fontHeight;
extern int currentfont;
extern const char* fonts[];


// DX9
extern IDirect3DDevice9Ex* p_Device;
extern ID3DXFont* pFontSmall;
extern ID3DXFont* pFontBig;
