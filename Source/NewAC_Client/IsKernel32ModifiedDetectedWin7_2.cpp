#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsKernel32ModifiedDetectedWin7_2()
{
	Kernel32 = (DWORD)GetModuleHandleA(kernel32dll);
	if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x00\x90\x90\xFF\x25\x00\x00\x00\x00\x90\x90\x90\x90\x90\x8B\xFF\x55\x8B\xEC\x5D\xEB\x05\x90\x90\x90\x90\x90", AC_CHEATENGINE_WIN7_MASK) != NULL) // CheatEngine
		return true;

	return false;
}