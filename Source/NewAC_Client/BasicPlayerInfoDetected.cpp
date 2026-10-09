#include "stdafx.h"
#include "SAC.h"

bool She3aAC::IsBasicPlayerInfoModified()
{

	if (!cPlayer->cEngine->GetCShellDLL())

		return false;
	DWORD BasicPlayer = *(DWORD*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_BASICPLAYERINFO);
	if (*(float*)(BasicPlayer + 0x7C) != 5.0f) // C4 Plant
		return true;
	if (*(float*)(BasicPlayer + 0x80) != 7.0f) // C4 Defuse
		return true;
	if (*(float*)(BasicPlayer + 0x84) != 300.0f) // C4 Max Distance
		return true;
	if (*(float*)(BasicPlayer + 0x8) != 0.5908f) // MovementWalkRate
		return true;
	if (*(float*)(BasicPlayer + 0xC) != 0.38028f) // MovementDuckWalkRate
		return true;

	return false;
}