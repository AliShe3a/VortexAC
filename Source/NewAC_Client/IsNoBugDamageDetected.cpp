#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsNoBugDamageEnabled()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	//if (strcmp((PCHAR)(CShell + OFF_CSHELL_NOZONEDAMAGE), AC_NoZoneDamage) != 0) // NoZoneDamage using 1 address
	//	return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_VISIBLEZONEINDEX), AC_VisibleZoneIndex) != 0) // VisibleZoneIndex
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_DAMAGEZONE), AC_DamageZone) != 0) // DamageZone
		return true;
	if (strcmp((PCHAR)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_DAMAGPERSEC), AC_DamagePerSec) != 0) // DamagePerSec
		return true;
	return false;
}