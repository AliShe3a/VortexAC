#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsNadeBypassed()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;
	if (!PatternCmp((PBYTE)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_NOFLASH), (PBYTE)"\x00\x98\xff\xff\xe9\x0d", AC_NOFLASH_MASK)) //NOFLASH
		return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_NOSMOKE), (PBYTE)"\x83\x7F\x0C\x00\x77\x08\x57", AC_NOSMOKE_MASK)) //NOSMOKE
	//	return true;
	//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_NOSMOKE_), (PBYTE)"\x83\x7C\x24\x18\x00\x74\x1B\x8B\xCE\xE8\x00\x00\x00\x00\xD9\xEE\x83\xEC\x08", AC_NOSMOKE_MASK_)) //NOSMOKE
	//	return true;
	return false;
}