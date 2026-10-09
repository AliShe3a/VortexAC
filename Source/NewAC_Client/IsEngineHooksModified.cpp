#include "stdafx.h"
#include "SAC.h"
#include "MemoryAddr.h"

bool She3aAC::IsEngineHooksModified()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	int GameStatus = *reinterpret_cast<int*>(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INAGMEADDR);
	if (!PatternCmp((PBYTE)(OFF_DIP_HOOK), (PBYTE)"\x8B\x80\x48\x01\x00\x00\x57\x51\x55\x52\x6A\x00\x6A\x04\x53\xFF\xD0\x85\xC0\x7C\x10", AC_DIP_HOOK_MASK)) // DIP Engine Hook 1
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_DIP_HOOK_), (PBYTE)"\x8B\x88\x48\x01\x00\x00\x55\x6A\x00\x53\x6A\x00\x6A\x00\x6A\x04\x57\xFF\xD1\x85\xC0\x7C\x00", AC_DIP_HOOK__MASK)) // DIP Engine Hook 1
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_PRESENT_ENGINE_HOOK), (PBYTE)"\x8B\x51\x00\x6A\x00\x6A\x00\x6A\x00\x50\xFF\xD2\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00", AC_PRESENT_ENGINE_HOOK_MASK)) // Present Engine Hook
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_ENDSCENE_ENGINE_HOOK), (PBYTE)"\x8B\x08\x8B\x91\x00\x00\x00\x00\x50\xFF\xD2\x85\xC0\x8D\x4C\x24\x00\x0F\x94\xC3\xE8\x00\x00\x00\x00\x8A\xC3\x8B", AC_ENDSCENE_ENGINE_HOOK_MASK)) // EndScene Engine Hook
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_DRAW_MENU_HOOK), (PBYTE)"\x8B\x35\x00\x00\x00\x00\x8B\xEE\xE8\x00\x00\x00\x00\x8B\x45\x00\x8B\x08\x8B\x91", AC_DRAWMENU_HOOK_MASK)) // Draw Menu (RAMLeague public) http://www.ramleague.net/threads/draw-menu-aimbot-remotekill.58397/
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_UNIVERSAL_HOOK), (PBYTE)"\x8B\x10\x8B\x92\x00\x00\x00\x00\x51\x6A\x0E\x50\xFF\xD2\x8B\x94\x24\x00\x00\x00\x00", AC_UNIVERSAL_HOOK_MASK)) //  Universal D3D Hook https://www.mpgh.net/forum/showthread.php?t=1367637
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(OFF_INTERSEGMENT_HOOK), (PBYTE)"\x5D\xC3\xCC\x55\x8B\xEC\x8B\x45\x0C\x50\x8B\x4D\x08\x51\x8B\x15\x00\x00\x00\x00", AC_INTERSEGMENT_HOOK_MASK)) //  5D C3 CC 55 8B EC 8B 45 0C 50 8B 4D 08 51 8B 15 ?? ?? ?? ??
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_HOOK_1), (PBYTE)"\x8B\x91\x00\x00\x00\x00\x6A\x01\xFF\xD2\x83\xC4\x04\x85\xC0", AC_CSHELL_1_HOOK_MASK)) //  CShell hook by PaoMfz
		return true;
	if (GameStatus == 1)
		Sleep(100);
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_FLIP_SCREEN_XCHEAT), (PBYTE)"\xFF\xD0\x8B\xCB\xE8\x55\x96\xFD\xFF\x8B\x0D", AC_FLIPSCREEN_MASK_XCHEAT)) //  XCheat Flip Screen
	//	return true;
	if (GameStatus == 1)
		Sleep(100);



	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_CALL), (PBYTE)"\x8B\x91\x00\x00\x00\x00\x6A\x00\xFF\xD2\x83\xC4\x04\x85\xC0", AC_FLIPSCREEN_MASK)) // FlipScreen https://github.com/biesigrr/phoenix-cf/blob/b803278fdd553b2d8bcabebd085a60d018f9f8d8/source/engine.cpp#L72
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (*(long*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_CALL + 1) < 0 || *(long*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_CALL + 1) > 0xC00000)
		return true;
	if (GameStatus == 1)
		Sleep(100);
	if (cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_CALL + *(DWORD*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_CALL + 1) + 5 != cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FLIPSCREEN_ORIG_ADDR)
		return true;

	return false;
}