#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsStringReloadEtcModified()
{

	if (!cPlayer->cEngine->GetCShellDLL())

		return false;


	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_RELOAD_STRING), AC_RELOAD) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_SELECT_STRING), AC_SELECT) != 0)
		return true;

	return false;


}