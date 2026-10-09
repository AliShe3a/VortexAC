#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsD3DModified()
{

	D3Dx9 = (DWORD)GetModuleHandleA(d3d9);
	//DWORD GameStatus = *(DWORD*)(GetCShell() + OFF_CSHELL_GAMESTATUS);
	if (D3Dx9 != NULL)
	{
		if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x6A\xFF\x68\x00\x00\x00\x00", AC_D3DMID_MASK) != NULL) // Waller D3D Midfunc hook	
			return true;
		if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x3B\xFB\x0F\x84\x00\x00\x00\x00", AC_D3DMID2E9_MASK) != NULL) // Ramo D3D Hack
			return true;
		if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x51\x8B\x4D\x08\x33\xC0", AC_WALLHACKV18_MASK) != NULL) // Wallhack v1.8
			return true;
		if (FindPatternVideo(d3d9, "\xB8\x00\x00\x00\x00\xFF\xE0\x68\x00\x00\x00\x00\x64\xA1", AC_ALITAREK_MASK) != NULL) // Chinese D3D Hack 
			return true;

		//if (IsWindows8OrGreater())
		//{
		//	if (FindPatternVideo(d3d9, "\xCC\xFF\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00", AC_D3D_PINOY_MASK) != NULL) // Pinoy Cheat
		//		return true;
		//	/*if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x83\xE4\xF8\x51\x53\x8B\x5D\x08\x56\xBE\x00\x00\x00\x00\x57", AC_CHINESED3D_MASK) != NULL) // Chinese D3D Hack
		//		return true;*/
		//	if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x83\xE4\xF8\x51\x53\x8B\x5D", AC_CHINESED3D2_MASK) != NULL) // Chinese D3D Hack
		//		return true;
		//	if (FindPatternVideo(d3d9, "\xCC\xE9\x00\x00\x00\x00\x51\xE8\x00\x00\x00\x00\x85\xC0", AC_WallHackRanger_18_MASK_3) != NULL) // Wallhack v1.8 RanGer CF
		//		return true;
		//}
		//if (IsWindows7OrGreater() && !IsWindows8OrGreater())
		//{
		//	if (FindPatternVideo(d3d9, "\x90\x90\xE9\x00\x00\x00\x00\x51\xE8\x00\x00\x00\x00\x85\xC0", AC_WallHackRanger_18_MASK_2) != NULL) // Wallhack v1.8 RanGer CF
		//		return true;
		//}
	}

	return false;
}

bool She3aAC::IsD3DModified2()
{

	D3Dx9 = (DWORD)GetModuleHandleA(d3d9);
	if (D3Dx9 != NULL)
	{
		if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x8B\x4D\x0C\x83\xEC\x00\x53\x56\x57\x8B\x7D", AC_BLUECHAMS_HOOK_WINDOWS10_MASK) != NULL) // Blue Chams hook
			return true;
		if (FindPatternVideo(d3d9, "\xE9\x00\x00\x00\x00\x8B\x45\x00\x53", AC_BLUECHAMS_HOOK_WINDOWS7_MASK) != NULL) // Blue Chams hook Windows 7
			return true;

		//if (IsWindows7OrGreater() && !IsWindows8OrGreater())
		//{
		//	if (FindPatternVideo(d3d9, "\x90\xE9\x00\x00\x00\x00\x56\x57", AC_THE_GEEK_HOOK) != NULL) // The Geek Windows 7
		//		return true;
		//}

		//if (IsWindows8OrGreater())
		//{
		//	if (FindPatternVideo(d3d9, "\xCC\xE9\x00\x00\x00\x00\x83\xE4", AC_THE_GEEK_HOOK_WIN10) != NULL) // TheGeek Windows 10
		//		return true;
		//}
	}
	return false;
}