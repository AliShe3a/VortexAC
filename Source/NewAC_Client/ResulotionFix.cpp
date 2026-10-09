#include <map>
#include "SAC.h"
#include "VortexHooks.h"
#include "ResulotionFix.h"



UserSettings g_Settings = { 3, 1, 1280, 720, 1.25f, 0.9375f, 1.25f, 0.9375f , false };


void LoadUserSettings() {
	char path[MAX_PATH];


	int resIdx = 3; // 1280x720
	int sMode = 1;  // Window Mode

	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
		std::string fullPath = std::string(path) + "\\Cross Fire\\System.ini";

		resIdx = GetPrivateProfileIntA("Graphic", "AspectRatio", 3, fullPath.c_str());
		sMode = GetPrivateProfileIntA("Graphic", "ScreenMode", 1, fullPath.c_str());
	}

	int tableSize = sizeof(g_ResolutionTable) / sizeof(Resolution);
	if (resIdx < 0 || resIdx >= tableSize) {
		resIdx = 1; // Fallback to 1024x768
	}

	g_Settings.resolutionIndex = resIdx;
	g_Settings.screenMode = sMode;
	g_Settings.targetWidth = g_ResolutionTable[resIdx].width;
	g_Settings.targetHeight = g_ResolutionTable[resIdx].height;

	g_Settings.scaleX = (float)g_Settings.targetWidth / 1024.0f;
	g_Settings.scaleY = (float)g_Settings.targetHeight / 768.0f;

	g_Settings.mouseScaleX = g_Settings.scaleX;
	g_Settings.mouseScaleY = g_Settings.scaleY;

	//printf("[CF_INIT] Smart Settings Loaded: Index %d -> %dx%d | Mode: %d\n",
	//	resIdx, g_Settings.targetWidth, g_Settings.targetHeight, sMode);
	//printf("[CF_INIT] Computed Scales: X=%.4f, Y=%.4f\n", g_Settings.scaleX, g_Settings.scaleY);
}

BOOL WINAPI Hooked_InternalGetCursorPos(LPPOINT lpPoint) {
	BOOL result = GameHooks.oGetCursorPos(lpPoint);

	if (IsInGame() || g_Settings.bypassScaling || !result || !lpPoint || !g_hGameWindow)
		return result;

	RECT rect;
	if (GetWindowRect(g_hGameWindow, &rect)) {
		float relX = (float)(lpPoint->x - rect.left);
		float relY = (float)(lpPoint->y - rect.top);

		lpPoint->x = (LONG)(rect.left + (relX / g_Settings.mouseScaleX));
		lpPoint->y = (LONG)(rect.top + (relY / g_Settings.mouseScaleY));
	}
	return result;
}


int __fastcall HookedChangeDisplayMode(void* pThis, void* edx, int mode, int eScreenMode, int bCamera, int bSameCheck) {
	HMODULE hCShell = GetModuleHandleA("CShell.dll");

	if (GameHooks.pSprite) GameHooks.pSprite->OnLostDevice();


	if (hCShell) {
		int liveMode = *(int*)((uintptr_t)hCShell + 0x166ACA0);
		int liveScreenMode = *(int*)((uintptr_t)hCShell + 0x166ACA8);
		if (liveMode != g_Settings.resolutionIndex || liveScreenMode != g_Settings.screenMode) {
			g_Settings.resolutionIndex = liveMode;
			g_Settings.screenMode = liveScreenMode;
			int tableSize = sizeof(g_ResolutionTable) / sizeof(Resolution);
			int safeIdx = (liveMode >= 0 && liveMode < tableSize) ? liveMode : 1;
			g_Settings.targetWidth = g_ResolutionTable[safeIdx].width;
			g_Settings.targetHeight = g_ResolutionTable[safeIdx].height;
			g_Settings.scaleX = (float)g_Settings.targetWidth / 1024.0f;
			g_Settings.scaleY = (float)g_Settings.targetHeight / 768.0f;
			g_Settings.mouseScaleX = g_Settings.scaleX;
			g_Settings.mouseScaleY = g_Settings.scaleY;
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
				std::string iniPath = std::string(path) + "\\Cross Fire\\System.ini";
				WritePrivateProfileStringA("Graphic", "ScreenMode", std::to_string(g_Settings.screenMode).c_str(), iniPath.c_str());
				WritePrivateProfileStringA("Graphic", "AspectRatio", std::to_string(g_Settings.resolutionIndex).c_str(), iniPath.c_str());
			}
		}
	}
	if (mode == g_Settings.resolutionIndex && eScreenMode == g_Settings.screenMode) {
		g_Settings.bypassScaling = true;
	}
	else {
		g_Settings.bypassScaling = false;
		mode = g_Settings.resolutionIndex;
		eScreenMode = g_Settings.screenMode;
	}

	int ret = GameHooks.oChangeDisplayMode(pThis, edx, mode, eScreenMode, bCamera, bSameCheck);

	if (GameHooks.pSprite) GameHooks.pSprite->OnResetDevice();

	return ret;
}


HRESULT __stdcall HookedSetViewport(IDirect3DDevice9* pDevice, const D3DVIEWPORT9* pViewport) {
	// 1. Hook Initialization Logic
	if (oWndProc == nullptr) {
		D3DDEVICE_CREATION_PARAMETERS params;
		if (SUCCEEDED(pDevice->GetCreationParameters(&params))) {
			InitInputHooks(params.hFocusWindow);
		}
	}
	if (!IsInGame() && !g_Settings.bypassScaling && pViewport) {
		float sX = (float)g_Settings.targetWidth / LOBBY_BASE_WIDTH;
		float sY = (float)g_Settings.targetHeight / LOBBY_BASE_HEIGHT;
		D3DVIEWPORT9 newVP = *pViewport;
		bool isOriginalBase = (pViewport->Width == LOBBY_BASE_WIDTH && pViewport->Height == LOBBY_BASE_HEIGHT);
		bool isAlreadyTarget = (pViewport->Width == g_Settings.targetWidth && pViewport->Height == g_Settings.targetHeight);
		if (isOriginalBase || isAlreadyTarget) {
			newVP.X = 0;
			newVP.Y = 0;
			newVP.Width = (DWORD)g_Settings.targetWidth;
			newVP.Height = (DWORD)g_Settings.targetHeight;
		}
		else {
			newVP.X = (DWORD)(pViewport->X * sX);
			newVP.Y = (DWORD)(pViewport->Y * sY);
			newVP.Width = (DWORD)(pViewport->Width * sX);
			newVP.Height = (DWORD)(pViewport->Height * sY);
		}
		newVP.MinZ = 0.0f;
		newVP.MaxZ = 1.0f;
		if (newVP.X + newVP.Width > g_Settings.targetWidth) {
			newVP.Width = g_Settings.targetWidth - newVP.X;
		}
		if (newVP.Y + newVP.Height > g_Settings.targetHeight) {
			newVP.Height = g_Settings.targetHeight - newVP.Y;
		}

		return GameHooks.oSetViewport(pDevice, &newVP);
	}

	return GameHooks.oSetViewport(pDevice, pViewport);
}

HRESULT __stdcall HookedDrawPrimitiveUP(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {

	if (IsInGame() || g_Settings.bypassScaling || !pVertexStreamZeroData) {
		return GameHooks.oDrawPrimitiveUP(pDevice, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	DWORD fvf = 0;
	if (FAILED(pDevice->GetFVF(&fvf)) || !(fvf & D3DFVF_XYZRHW)) {
		return GameHooks.oDrawPrimitiveUP(pDevice, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	UINT numVertices = 0;
	switch (PrimitiveType) {
	case D3DPT_TRIANGLELIST:  numVertices = PrimitiveCount * 3; break;
	case D3DPT_TRIANGLESTRIP: numVertices = PrimitiveCount + 2; break;
	case D3DPT_TRIANGLEFAN:   numVertices = PrimitiveCount + 2; break;
	case D3DPT_LINELIST:      numVertices = PrimitiveCount * 2; break;
	case D3DPT_LINESTRIP:     numVertices = PrimitiveCount + 1; break;
	case D3DPT_POINTLIST:     numVertices = PrimitiveCount;     break;
	default: return GameHooks.oDrawPrimitiveUP(pDevice, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	static std::vector<BYTE> tempBuffer;
	size_t dataSize = numVertices * VertexStreamZeroStride;

	if (tempBuffer.capacity() < dataSize) tempBuffer.reserve(dataSize * 2);
	tempBuffer.resize(dataSize);

	memcpy(tempBuffer.data(), pVertexStreamZeroData, dataSize);

	float sX = g_Settings.mouseScaleX;
	float sY = g_Settings.mouseScaleY;

	for (UINT i = 0; i < numVertices; i++) {
		UI_VERTEX* v = (UI_VERTEX*)(tempBuffer.data() + (i * VertexStreamZeroStride));
		v->x = (v->x - 0.5f) * sX + 0.5f;
		v->y = (v->y - 0.5f) * sY + 0.5f;
	}

	return GameHooks.oDrawPrimitiveUP(pDevice, PrimitiveType, PrimitiveCount, tempBuffer.data(), VertexStreamZeroStride);
}
