#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsKernel32ModifiedDetected()
{
	Kernel32 = (DWORD)GetModuleHandleA(kernel32dll);
	if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x5D\xFF\x00\x00\x00\x00\xCC", AC_QUERYERPERFORMANCECOUNTER_MASK) != NULL) // QueryPerformanceCounter
		return true;
	if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x00\x00\x00\x59\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC", AC_TICKCOUNT_MASK) != NULL) // TickCount
		return true;
	if (FindPatternVideo(kernel32dll, "\xE9\x00\x00\x00\x00\x51\x53\x56\xBA\x00\x00\x00\x00\x57", AC_CHEATENGINE_MASK) != NULL) // TickCount
		return true;
	return false;
}
