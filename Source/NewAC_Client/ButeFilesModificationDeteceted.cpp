#include "stdafx.h"
#include "SAC.h"


bool She3aAC::ButeFilesModificatioNDetected()
{
	DWORD FastSwitchModule = *(DWORD*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_WEAPONMGR_BYPASS_);

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	if (FastSwitchModule != NULL)
	{
		for (int i = 0; i < 4096; i++)
		{
			DWORD SemiFastSwitch = *(DWORD*)(FastSwitchModule + (4 * i));
			if (SemiFastSwitch != NULL)
			{
				if (*(float*)(SemiFastSwitch + VALUE_NO_SWITCH) > 1.0f && i != 2968)
					return true;
				if (*(float*)(SemiFastSwitch + VALUE_NO_SWITCH_) > 2.0f && i != 2968)
					return true;
				if (*(float*)(SemiFastSwitch + OFF_RELOAD_WEAPMGR) > 1.65f)
					return true;
				if (*(float*)(SemiFastSwitch + OFF_CHANGEWEAPONANIMRATIO) > 1.86f && i != 823 && i != 1081 && i != 1494)
					return true;
				if (*(float*)(SemiFastSwitch + VALUE_FIREANIMMULTIPLER_VALUE) > 1.75f && (*(BYTE*)(SemiFastSwitch + 0x2) == 4 && i != 2066))
					return true;
				if (*(float*)(SemiFastSwitch + VALUE_FIREANIMMULTIPLER_VALUE) > 7.0f && (*(BYTE*)(SemiFastSwitch + 0x2) != 4))
					return true;
				if (*(float*)(SemiFastSwitch + VALUE_NO_GRENADE) < 118.0f && (*(BYTE*)(SemiFastSwitch + 0x2) == 6) && (*(BYTE*)(SemiFastSwitch + 0xBE4) == 3) && i != 1615 && i != 423 && i != 1071 && i != 1161 && i != 1591 && i != 2410 && i != 2883 && i != 2977 && i != 3478 && i != 3481 && i != 3484 && i != 3490)
					return true;

			}

			Sleep(1);
		}

	}
	return false;
}