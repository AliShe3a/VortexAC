#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsMTPPerfectRecoil()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_CROSSHAIRRATIOOLD), (PBYTE)"\x7B\x10\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xF8\x06", AC_CROSSHAIRRATIOOLD_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_CROSSHAIRRATIONEW), (PBYTE)"\x7B\x10\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xF8\x06", AC_CROSSHAIRRATIONEW_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_PERTRUBMINOLD), (PBYTE)"\x0F\x85\x00\x00\x00\x00\xD8\xD2\xDF\xE0\xDD\xDA\xF6\xC4\x41\x0F\x85\x00\x00\x00\x00", AC_PERTRUBMINOLD_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_PERTRUBMINNEW), (PBYTE)"\x0F\x85\x00\x00\x00\x00\xD8\xD2\xDF\xE0\xDD\xDA\xF6\xC4\x41\x0F\x85\x00\x00\x00\x00", AC_PERTRUBMINNEW_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_SHOTREACTYAWOLD), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x53\xBB\x00\x00\x00\x00\x39\x9D\x00\x00\x00\x00\x56\x75\x0A", AC_SHOTREACTYAWOLD_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_SHOTREACTYAWNEW), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x53\xBB\x00\x00\x00\x00\x39\x9D\x00\x00\x00\x00\x56\x75\x0A", AC_SHOTREACTYAWNEW_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_SHOTREACTPITCHOLD), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x85\xC0\x74\x24", AC_SHOTREACTPITCHOLD_MASK))
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_SHOTREACTPITCHNEW), (PBYTE)"\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x85\xC0\x74\x24", AC_SHOTREACTPITCHNEW_MASK))
	//	return true;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_FAST_FIRE), (PBYTE)"\x00\x00\x00\x00\x00\x40\x8F\x40\x00\x00\x00\x00\x00\x40\xBF\x40", AC_FAST_FIRE_MODIFIED))
		return true;
	return false;
}