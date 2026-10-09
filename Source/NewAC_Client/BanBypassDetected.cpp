#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsBanPacketBypassed()
{
	

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	if (!PatternCmp((PBYTE)(OFF_BYPASS_DC_BAN), (PBYTE)"\x74\x14\x8D\x8C\x24\x00\x00\x00\x00\x51\x55", AC_BYPASS_DC_MASK))
		return true;
	if (!PatternCmp((PBYTE)(OFF_BYPASS_MEMORYSCANENGINE), (PBYTE)"\x75\x0B\x53\xE8\x00\x00\x00\x00", AC_BYPASS_MEMORYSCAN))
		return true;
	if (!PatternCmp((PBYTE)(OFF_ADDR_CHECK_3_), (PBYTE)"\xE8\x00\x00\x00\x00\x5D\xC2\x04\x00\xCC", AC_ADDR_CHECK_3_))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_ADDR_CHECK_4), (PBYTE)"\x74\x1E\x8B\xB0\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11\x8B\x80\x00\x00\x00\x00\x8B\x92\x00\x00\x00\x00\x56\x50\xFF\xD2\x5F\x5E", AC_ADDR_CHECK_4))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_ADDR_CHECK_3), (PBYTE)"\xE8\x00\x00\x00\x00\x5D\xC2\x04\x00\xCC", AC_ADDR_CHECK_3_))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_ADDR_CHECK_2), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x80\x7C\x24\x04\x01\x74\x13\x8B\x01\x8B\x90\x00\x00\x00\x00\xFF\xD2\x83\xF8\x02\x0F\x85\x00\x00\x00\x00\x56\x57\x33\xFF", AC_ADDR_CHECK_2))
		return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_ADDR_CHECK_1), (PBYTE)"\x74\x6E\x8B\xB0\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11\x8B\x80\x00\x00\x00\x00\x8B\x92\x00\x00\x00\x00\x56\x50\xFF\xD2\xEB\x4E\x3B\xCF\x74\x53", AC_ADDR_CHECK_1))
		return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_CLIENT_ERROR_30_12), (PBYTE)"\x8B\x94\x8E\x00\x00\x00\x00\xD9\x86\x00\x00\x00\x00\xD9\x84\x97\x00\x00\x00\x00\x8D\x8C\x97\x00\x00\x00\x00\xDE\xD9\xDF\xE0", AC_CLIENT_ERROR_30_12_MASK))
	//	return true;
	return false;
}


bool She3aAC::IsCHBypassed()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_CH_BYPASS), (PBYTE)"\x74\x00\x8B\xB0\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11", AC_CH_BYPASS_MASK))
		return true;
	return false;
}