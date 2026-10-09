#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsGlowHack()
{
	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	if (*(float*)(OFF_GLOW_THICK) != 0.0f)
		return true;
	/*if (!PatternCmp((PBYTE)(CShell + OFF_GLOW_ENABLE), (PBYTE)"\xC7\x46\x00\x65\x00\x00\x00\xEB\x10\x84\xDB\x75\x09\xC7\x46\x00\x67\x00\x00\x00", AC_GLOW_ENABLE_MASK))
	return true;

	typedef void(WINAPI* oGlowESP)(bool value);

	oGlowESP pSetGlowColor = (oGlowESP)((DWORD)CShell + OFF_GLOW_ENABLE);
	pSetGlowColor(true);*/
	return false;
}
