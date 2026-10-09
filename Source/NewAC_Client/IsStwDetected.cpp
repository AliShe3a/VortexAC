#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsStw()
{

	if (!cPlayer->cEngine->CrossFire)
		return false;

	DWORD pSTWMgr = *(DWORD*)(cPlayer->cEngine->CrossFire + OFF_STW);
	if (pSTWMgr != NULL)
	{
		if (*(float*)((pSTWMgr + 0xC)) != 0.0f) //Shoot Through Wall (Remote)
			//CustomInfoBox("pSTWMgr Value: %f",*(float*)((OFF_STW + 0xC )));
			//*(float*)(OFF_STW + 0xC) = 100.0f;
			return true;
	}
	return false;
}