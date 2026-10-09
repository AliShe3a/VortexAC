#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")


struct UserSettings {
	int resolutionIndex;
	int screenMode;
	int targetWidth;
	int targetHeight;

	float scaleX;
	float scaleY;

	float mouseScaleX;
	float mouseScaleY;

	bool bypassScaling;
};

class ENGINE_HOOKS 
{
public:

	 ID3DXSprite* pSprite = nullptr;

	typedef  int(__fastcall* tChangeDisplayMode)(void* pThis, void* edx, int mode, int eScreenMode, int bCamera, int bSameCheck);
	typedef HRESULT(WINAPI* tSetViewport)(IDirect3DDevice9*, const D3DVIEWPORT9*);
	typedef HRESULT(WINAPI* EndScene_t)(IDirect3DDevice9* pDevice);
	typedef HRESULT(WINAPI* tDrawPrimitiveUP)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, CONST void*, UINT);
	typedef BOOL(WINAPI* tGetCursorPos)(LPPOINT lpPoint);
	typedef void(__stdcall* UpdatePlayerStats_t)(int a1);

	tChangeDisplayMode oChangeDisplayMode = nullptr;
	tSetViewport oSetViewport = nullptr;
	EndScene_t oEndScene = nullptr;
	tDrawPrimitiveUP oDrawPrimitiveUP = nullptr;
	 tGetCursorPos oGetCursorPos = nullptr;
	 UpdatePlayerStats_t UpdatePlayerStats = nullptr;
};



extern WNDPROC oWndProc;
extern HWND g_hGameWindow;
extern UserSettings g_Settings;


extern bool IsInGame();
extern void InitInputHooks(HWND hWnd);
extern void InitializeHooks();
extern  bool ExecuteAutoLogin();
extern ENGINE_HOOKS GameHooks;

extern HRESULT APIENTRY EndScene_hk(IDirect3DDevice9* pDevice);
extern LRESULT CALLBACK WndProc_hk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);