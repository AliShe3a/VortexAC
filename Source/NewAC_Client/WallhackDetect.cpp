#include "stdafx.h"
#include "SAC.h"


bool She3aAC::IsWallArrayModified()
{
	ObjectDll = (DWORD)GetModuleHandleA(Object_dll);

	if (*(DWORD*)(OFF_WALLARRAY + 0xB8) != 5) // See Ghost
		return true;
	if (*(DWORD*)(OFF_WALLARRAY + 0xA4) != 16777217) // Wall
		return true;
	if (*(DWORD*)(OFF_WALLARRAY_1) != 16777217) // Wall
		return true;
	if (*(DWORD*)(OFF_WALLARRAY_2) != 3) // WIREFRAME
		return true;
	if (*(DWORD*)((OFF_WALLARRAY + 0xA4) + 0x68) != 4) // Wall
		return true;
	//if (*(DWORD*)(OFF_WALLARRAY + 0x10) != 5) // FullBright
	//	return true;
	if (!PatternCmp((PBYTE)(OFF_WALLARRAY + 0xB8), (PBYTE)"\x05", "x"))
		return true;
	if (!PatternCmp((PBYTE)(OFF_WALLARRAY + 0xA4), (PBYTE)"\x01\x00\x00\x01\x01\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00", "xxxxxxxxxxxxxxxx"))
		return true;
	if (!PatternCmp((PBYTE)((OFF_WALLARRAY + 0xA4) + 0x68), (PBYTE)"\x04", "x"))
		return true;

	//CustomInfoBox("See Ghost (OFF_WALLARRAY_): %u",*(DWORD*)((OFF_WALLARRAY_ + 0xA) + 0xB8));
	//CustomInfoBox("See Ghost (OFF_WALLARRAY): %u",*(DWORD*)(OFF_WALLARRAY + 0xB8));
	return false;
}

void She3aAC::SetAntiWallBan()
{
	*(DWORD*)(OFF_WALLARRAY + 0x10) = 5;
	*(DWORD*)(OFF_WALLARRAY + 0xB8) = 5;
	*(DWORD*)(OFF_WALLARRAY + 0xA4) = 16777217;
	*(DWORD*)(OFF_WALLARRAY_1) = 16777217;
	*(DWORD*)(OFF_WALLARRAY_2) = 3;
	*(DWORD*)((OFF_WALLARRAY + 0xA4) + 0x68) = 4;
	memcpy((void*)(OFF_WALLARRAY + 0xA4), "\x01\x00\x00\x01\x01\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00", 16);
	memcpy((void*)(OFF_WALLARRAY + 0xB8), "\x05", 1);
	memcpy((void*)((OFF_WALLARRAY + 0xA4) + 0x68), "\x04", 1);
}