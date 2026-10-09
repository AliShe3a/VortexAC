#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsInvisibleCharacter()
{
	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	//make the xor..
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_IDLE), AC_MIDLE) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MWAIT0), AC_MWAIT0) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MCIDLE), AC_MCIDLE) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MRUN), AC_MRUN) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MRUNB), AC_MRUNB) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MRUNR), AC_MRUNR) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MRUNL), AC_MRUNL) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MWALK), AC_MWALK) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MWALKB), AC_MWALKB) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MWALKR), AC_MWALKR) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MWALKL), AC_MWALKL) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MCWALK), AC_MCWALK) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MCWALKB), AC_MCWALKB) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MCWALKR), AC_MCWALKR) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MCWALKL), AC_MCWALKL) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MJUMP), AC_MJUMP) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MHIT01), AC_MHIT01) != 0)
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_INVISIBLE_MHIT02), AC_MHIT02) != 0)
		return true;

	return false;
}