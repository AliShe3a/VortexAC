#include "SAC.h"
#include <windows.h>
#include "VortexHooks.h"
#include "ResulotionFix.h"


ENGINE_HOOKS GameHooks = ENGINE_HOOKS();
WNDPROC oWndProc = nullptr;
HWND g_hGameWindow = nullptr;




bool IsInGame() {
	DWORD hCShellBase = (DWORD)GetModuleHandleA("CShell.dll");
	if (!hCShellBase) return false;

	int status = *reinterpret_cast<int*>(hCShellBase + 0x016B3F58 + 0x7C);

	return (status == 1);
}


LRESULT CALLBACK WndProc_hk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		if (She3aAC::Instance) {
			She3aAC::Instance->VOIPData.isUIVisible = !She3aAC::Instance->VOIPData.isUIVisible;
		}
	}

	if (uMsg == WM_LBUTTONDOWN || (uMsg == WM_MOUSEMOVE && (wParam & MK_LBUTTON)))
	{
		if (She3aAC::Instance) {
			int rawX = LOWORD(lParam);
			int rawY = HIWORD(lParam);

			int mouseX = rawX;
			int mouseY = rawY;

			if (!g_Settings.bypassScaling && g_Settings.mouseScaleX > 0 && g_Settings.mouseScaleY > 0) {
				RECT rect;
				if (GetWindowRect(hWnd, &rect)) {
					mouseX = (int)(rawX / g_Settings.mouseScaleX);
					mouseY = (int)(rawY / g_Settings.mouseScaleY);
				}
			}

			She3aAC::Instance->HandleVOIPClick(mouseX, mouseY);

			if (uMsg == WM_LBUTTONDOWN && She3aAC::Instance->VOIPData.isUIVisible &&
				mouseX >= She3aAC::Instance->VOIPData.uiX &&
				mouseX <= She3aAC::Instance->VOIPData.uiX + She3aAC::Instance->VOIPData.uiWidth &&
				mouseY >= She3aAC::Instance->VOIPData.uiY &&
				mouseY <= She3aAC::Instance->VOIPData.uiY + She3aAC::Instance->VOIPData.uiHeight)
			{
				return true;
			}

			if (uMsg == WM_LBUTTONDOWN && mouseX >= She3aAC::Instance->VOIPData.iconX && mouseX <= She3aAC::Instance->VOIPData.iconX + She3aAC::Instance->VOIPData.iconW &&
				mouseY >= She3aAC::Instance->VOIPData.iconY && mouseY <= She3aAC::Instance->VOIPData.iconY + She3aAC::Instance->VOIPData.iconH) {
				return true;
			}
		}
	}

	if (oWndProc) {
		return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void InitInputHooks(HWND hWnd) {

	static bool initDone = false;
	if (initDone) return;
	g_hGameWindow = hWnd;
	oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG_PTR)WndProc_hk);

	HMODULE hCShell = GetModuleHandleA("CShell.dll");
	if (!hCShell) {
		//printf("[CF_ERROR] CShell.dll is not loaded yet!\n");
		return;
	}

	uintptr_t callInstructionAddr = (uintptr_t)hCShell + 0x252E9F;
	if (*(BYTE*)callInstructionAddr != 0xE8) {
		//printf("[CF_ERROR] Opcode mismatch at offset! Check IDA offset again.\n");
		return;
	}

	int32_t relOffset = *(int32_t*)(callInstructionAddr + 1);
	uintptr_t finalTarget = callInstructionAddr + 5 + relOffset;
	//printf("[CF_SUCCESS] Resolved Private Wrapper at: 0x%p\n", (void*)finalTarget);
	GameHooks.oGetCursorPos = (ENGINE_HOOKS::tGetCursorPos)finalTarget;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)GameHooks.oGetCursorPos, Hooked_InternalGetCursorPos);

	if (DetourTransactionCommit() == NO_ERROR) {
		//	printf("[CF_FIX] Direct Offset Hook is now ACTIVE!\n");
		initDone = true;
	}
	else {
		//	printf("[CF_ERROR] Failed to detour the resolved address.\n");
	}
}

bool IsValidFunction(uintptr_t address) {
	if (!address) return false;

	if (IsBadReadPtr((void*)address, 5)) return false;

	unsigned char* bytes = (unsigned char*)address;

	// printf("[DEBUG] Bytes: %02X %02X %02X\n", bytes[0], bytes[1], bytes[2]);

	// 1. Standard Prologue: PUSH EBP; MOV EBP, ESP (55 8B EC)
	if (bytes[0] == 0x55 && bytes[1] == 0x8B && bytes[2] == 0xEC) return true;

	// 2. ThisCall Optimized Prologue: PUSH EBP; MOV EBP, ECX (55 8B E9) 
	if (bytes[0] == 0x55 && bytes[1] == 0x8B && bytes[2] == 0xE9) return true;

	// 3. Hotpatchable Header: MOV EDI, EDI; PUSH EBP; MOV EBP, ESP
	if (bytes[0] == 0x8B && bytes[1] == 0xFF && bytes[2] == 0x55) return true;

	// 4. Direct SUB ESP (e.g., 81 EC or 83 EC) - Common in optimized code
	if ((bytes[0] == 0x81 || bytes[0] == 0x83) && bytes[1] == 0xEC) return true;

	return false;
}

__declspec(noinline) IDirect3DDevice9* GetDevice(uintptr_t d3drefpointer) {
	if (IsBadReadPtr((void*)d3drefpointer, sizeof(DWORD))) return NULL;

	DWORD* dwPointer = (DWORD*)d3drefpointer;
	if (!dwPointer || !*dwPointer) return NULL;

	if (IsBadReadPtr((void*)*dwPointer, sizeof(DWORD))) return NULL;
	DWORD* dwPointerTwo = (DWORD*)*dwPointer;
	if (!dwPointerTwo || !*dwPointerTwo) return NULL;

	if (IsBadReadPtr((void*)*dwPointerTwo, sizeof(DWORD))) return NULL;
	DWORD* dwPointerThree = (DWORD*)*dwPointerTwo;
	if (!dwPointerThree || !*dwPointerThree) return NULL;

	return (IDirect3DDevice9*)*dwPointerThree;
}

void InitializeHooks() {
	LoadUserSettings();

	HMODULE hGame = GetModuleHandleA("crossfire.exe");
	if (!hGame) { printf("[CF_ERROR] Crossfire.exe not found!\n"); return; }

	uintptr_t pGameDevicePtr = (uintptr_t)hGame + 0xFBB5C;
	IDirect3DDevice9* g_pd3dDevice = nullptr;

	bool isLogicHooked = false;
	bool isRenderHooked = false;
	int retry = 0;

	//printf("[CF_HOOK] Starting Initialization...\n");

	while ((!isLogicHooked || !isRenderHooked) && retry < 2000) {

		// --- Part A: CShell Hooks ---
		if (!isLogicHooked) {
			HMODULE hCShell = GetModuleHandleA("CShell.dll");
			if (hCShell) {

				uintptr_t addr_ChangeDisplayMode = (uintptr_t)hCShell + 0xA3E30;

				if (IsValidFunction(addr_ChangeDisplayMode)) {
					GameHooks.oChangeDisplayMode = (ENGINE_HOOKS::tChangeDisplayMode)addr_ChangeDisplayMode;

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());
					DetourAttach(&(PVOID&)GameHooks.oChangeDisplayMode, HookedChangeDisplayMode);
					if (DetourTransactionCommit() == NO_ERROR) {
						isLogicHooked = true;
						//printf("[CF_HOOK] Logic Hooks Applied.\n");
					}
				}
			}
		}

		// --- Part B: Render Hooks ---
		if (!isRenderHooked) {
			if (!g_pd3dDevice) g_pd3dDevice = GetDevice(pGameDevicePtr);

			if (g_pd3dDevice) {
				uintptr_t* vtable = *(uintptr_t**)g_pd3dDevice;
				if (vtable) {

					// DrawIndexedPrimitiveUP هو رقم 82
					// SetViewport هو رقم 47
					GameHooks.oDrawPrimitiveUP = (ENGINE_HOOKS::tDrawPrimitiveUP)vtable[83];
					GameHooks.oSetViewport = (ENGINE_HOOKS::tSetViewport)vtable[47];
					GameHooks.oEndScene = (ENGINE_HOOKS::EndScene_t)vtable[42];

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());
					DetourAttach(&(PVOID&)GameHooks.oSetViewport, HookedSetViewport);
					DetourAttach(&(PVOID&)GameHooks.oDrawPrimitiveUP, HookedDrawPrimitiveUP);
					DetourAttach(&(PVOID&)GameHooks.oEndScene, EndScene_hk); 

					if (DetourTransactionCommit() == NO_ERROR) {
						isRenderHooked = true;
						//printf("[CF_HOOK] Render Hooks Applied (Viewport + DrawScaler).\n");
					}
				}
			}
		}


		if (isLogicHooked && isRenderHooked) break;
		Sleep(100);
		retry++;
	}
}