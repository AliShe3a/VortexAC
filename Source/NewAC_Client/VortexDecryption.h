#include <map>
#include <windows.h>
#include <string>
#include "xor.h"
#pragma once
// --- LithTech Constants ---
#define LT_OK 0
#define VM_HEAVY_START 
#define VM_HEAVY_END

// --- Definitions & Types ---
typedef unsigned int uint32;
const std::string SECRET_KEY = _xor("X@X#X$She3a!-*0*-!Was!-*0*-!Here$X#X@X").c_str();
const std::string HEADER_SIG = _xor("0xVortexEncryption!_X").c_str();
const std::string FOOTER_SIG = _xor("0xVortexEncryption!_X_E").c_str();


class ENGINE_ENCRYPTION 
{
public:
	typedef HANDLE(WINAPI* CreateFileAFn)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
	typedef BOOL(WINAPI* ReadFileFn)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	typedef BOOL(WINAPI* CloseHandleFn)(HANDLE);

	ReadFileFn    oReadFile = nullptr;
	CloseHandleFn oCloseHandle = nullptr;
	CreateFileAFn oCreateFileA = nullptr;


};

extern ENGINE_ENCRYPTION EncryptionEngine;

extern bool VortexEncryption(DWORD_PTR hCrossfire);