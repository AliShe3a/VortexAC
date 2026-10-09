#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsSuperKill()
{

	if (!cPlayer->cEngine->GetCShellDLL())
		return false;

	DWORD pHeadShotMgr = *(DWORD*)(cPlayer->cEngine->GetCShellDLL() + OFF_CSHELL_MODELNODE);
	if (pHeadShotMgr != NULL)
	{
		if (*(float*)((pHeadShotMgr + 0x38) + (0x9c)) != 17.0f)
			return true;
		if (*(float*)((pHeadShotMgr + 0x38 + (1 * 4)) + (0x9c)) != 17.0f)
			return true;
		if (*(float*)((pHeadShotMgr + 0x38 + (2 * 4)) + (0x9c)) != 17.0f)
			return true;
		if (*(float*)((pHeadShotMgr + 0x38 + (3 * 4)) + (0x9c)) != 1.0f)
			return true;
		//if (!PatternCmp((PBYTE)(CShell + OFF_CSHELL_MODEL_NODE_1), (PBYTE)"\xC7\x46\x00\x65\x00\x00\x00\xEB\x10\x84\xDB\x75\x09\xC7\x46\x00\x67\x00\x00\x00", AC_GOLDHEADSHOT_MASK))
		//	return true;

		//for (int i = 0; i < 4096; i++)
		//{
		//	
		//CustomInfoBox("SuperKill: %f, %u",(*(float*)((pHeadShotMgr + 0x38 + (i * 4)) + (0x9c)),i));

		//}
	}

	return false;
}