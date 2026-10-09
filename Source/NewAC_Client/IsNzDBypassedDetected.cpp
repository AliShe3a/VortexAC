#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsNzDBypassed()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_1), (PBYTE)"\x74\x1C\x56\x8B\x71\x00\x2B\xF0\xB8\x00\x00\x00\x00\xF7\xEE", AC_CSHELL_NZD_1))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_2), (PBYTE)"\x75\x04\x33\xC0\x5D\xC3\x53\x56\x57\x33\xFF\x85\xED\x76\x63\x8B\x40\x04", AC_CSHELL_NZD_2))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_3), (PBYTE)"\x75\x07\x33\xC0\xE9\x00\x00\x00\x00\x56\x8D\x4C\x24\x1C", AC_CSHELL_NZD_3))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_4), (PBYTE)"\x74\x1A\x56\x8B\x71\x00\x2B\xF0\xB8\x00\x00\x00\x00\xF7\xEE", AC_CSHELL_NZD_1))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_5), (PBYTE)"\x74\x28\x56\x8B\x71\x00\x2B\xF0\xB8\x00\x00\x00\x00\xF7\xEE", AC_CSHELL_NZD_1))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NZD_6), (PBYTE)"\x74\x3E\x8B\x5C\x24\x14\x8B\x2D\x00\x00\x00\x00\xEB\x03", AC_CSHELL_NZD_6))
		return true;
	if (!PatternCmp((PBYTE)(OFF_NZD_2_), (PBYTE)"\x75\x04\x33\xC0\x5D\xC3\x53\x56\x57\x33\xFF\x85\xED\x76\x63\x8B\x40\x04", AC_CSHELL_NZD_2))
		return true;
	if (!PatternCmp((PBYTE)(OFF_NZD_3_), (PBYTE)"\x75\x07\x33\xC0\xE9\x00\x00\x00\x00\x56\x8D\x4C\x24\x1C", AC_CSHELL_NZD_3))
		return true;
	if (!PatternCmp((PBYTE)(OFF_NZD_4_), (PBYTE)"\x74\x3E\x8B\x5C\x24\x14\x8B\x2D\x00\x00\x00\x00\xEB\x03", AC_CSHELL_NZD_6))
		return true;
	return false;
}