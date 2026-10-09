#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsAntiShakeScreen()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_CLIENT_ANTI_SHAKE), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x8B\x4C\x24\x20\xD9\x44\x24\x1C\x8B\x54\x24\x18", AC_CLIENT_ANTI_SHAKE_MASK))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_CLIENT_ANTI_SHAKE_), (PBYTE)"\x0F\x84\x00\x00\x00\x00\xD9\x44\x24\x1C\x8B\x54\x24\x10\x8B\x7C\x24\x0C", AC_CLIENT_ANTI_SHAKE_MASK))
		return true;
	/*	if (*(float*)((CShell + OFF_CSHELL_CLIENT_ANTI_SHAKE__)) == 274833521858609940000.000000f)
			return true;*/
			//if (*(DWORD*)((CShell + OFF_CSHELL_CLIENT_ANTI_SHAKE__)) != 1634623821)
			//	return true;
			//CustomInfoBox("AntiShake Value (DWORD): %u",(*(DWORD*)((CShell + OFF_CSHELL_CLIENT_ANTI_SHAKE__))));
	return false;
}