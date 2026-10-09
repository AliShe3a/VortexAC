#pragma once
#include <windows.h>
#include <Shlobj_core.h>

#define LOBBY_BASE_WIDTH 1024.0f
#define LOBBY_BASE_HEIGHT 768.0f


struct Resolution {
	int width;
	int height;
};

const Resolution g_ResolutionTable[] = {
	{ 800, 600 },   // Index 0
	{ 1024, 768 },  // Index 1
	{ 1280, 1024 }, // Index 2
	{ 1280, 720 },  // Index 3 (Wide)
	{ 1280, 800 },  // Index 4 (Wide)
	{ 1366, 768 },  // Index 5
	{ 1440, 900 },  // Index 6 (Wide)
	{ 1600, 900 },  // Index 7 (Wide)
	{ 1680, 1050 }, // Index 8 (Wide)
	{ 1920, 1080 }  // Index 9 (Wide)
};

struct UI_VERTEX {
	float x, y, z, rhw;
	DWORD color;
	float u, v;
};

extern void LoadUserSettings();


extern int __fastcall HookedChangeDisplayMode(void* pThis, void* edx, int mode, int eScreenMode, int bCamera, int bSameCheck);
extern BOOL WINAPI Hooked_InternalGetCursorPos(LPPOINT lpPoint);
extern HRESULT __stdcall HookedSetViewport(IDirect3DDevice9* pDevice, const D3DVIEWPORT9* pViewport);
extern HRESULT __stdcall HookedDrawPrimitiveUP(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);