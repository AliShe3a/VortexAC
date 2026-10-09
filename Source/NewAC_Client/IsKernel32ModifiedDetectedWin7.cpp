#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsKernel32ModifiedDetectedWin7()
{
	DWORD Kernel32 = (DWORD)GetModuleHandleA(kernel32dll);
	/*if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x00\x90\x90\xFF\x25\x00\x00\x00\x00", AC_QUERYPERFORMANCECOUNTER_WIN7_MASK) != NULL) // QueryPerformanceCounter
		MessageBoxA(NULL, "QueryPerformanceCounter", "QueryPerformanceCounter", MB_OK);
		//return true;*/
	if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x00\x90\x90\xFF\x25\x00\x00\x00\x00\x90\x90\x90\x90\x90", AC_TICKCOUNT_WIN7_MASK) != NULL) // TickCount
		//MessageBoxA(NULL, "TickCount", "TickCount", MB_OK);
		return true;

	return false;
}