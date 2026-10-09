#include "stdafx.h"
#include "SAC.h"

bool She3aAC::ClientErrorBypassDetected()
{
	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_CLIENT_ERROR_28_3), (PBYTE)"\x0F\x84\xF7\x01\x00\x00\x56\x50\xE8\x46\xDF\xD2", AC_CLIENT_ERROR_28_3_MASK))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_CLIENT_ERROR_31_0), (PBYTE)"\x0F\x84\xDB\x03", AC_CLIENT_ERROR_31_0_MASK))
		return true;

	return false;
}
