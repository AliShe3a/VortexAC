#include "SAC.h"

#define _TRUNCATE ((size_t)-1)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

//#include <windows.h>
//#include <iostream>
//#include "MemoryAddr.h"
//#include "xor2.h"
#include "hidemodule.h"
#include "process.h"
#include "MemoryAddr.h"
#include "ErrorCodes.h"

She3aAC* ac = NULL;
HANDLE acHandle = NULL;

void OpenConsole()
{
	AllocConsole();
	FILE* pConsole;
	freopen_s(&pConsole, "CONIN$", "r", stdin);
	freopen_s(&pConsole, "CONOUT$", "w", stdout);
	freopen_s(&pConsole, "CONOUT$", "w", stderr);
	printf("SHE3A AC 2.0 HAS BEEN LOADED SUCCESSFULLY\n");
}

void HandleAntiCheat(LPVOID)
{

	printf("[+] Starting AntiCheat Logic...\n");

	if (!She3aAC::Instance)
		She3aAC::Instance = new She3aAC();

	//printf("[+] She3aAC Instance Created at: 0x%p\n", She3aAC::Instance);

	if (She3aAC::Instance)
	{

	//	printf("[+] Initializing AntiCheat...\n");
		if (!She3aAC::Instance->Init())
		{
			She3aAC::Instance->ReportError(ANTICHEAT_NOT_INITIALIIZED);
			return;
		}
		She3aAC::Instance->Run();
	}
}

void OnProcessExit() {

	if (She3aAC::Instance)
	She3aAC::Instance->SendLogOut();

	return;
}

extern "C" __declspec(dllexport) void Init() {
		const char* she3a = _xor("GetDunnkedBitchxD").c_str();
	printf(_xor("1234").c_str());
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{

		//OpenConsole();

		// ANTIDEBUG & DUMP //
		DisableThreadLibraryCalls(hModule);
		HideModule(hModule);

		g_FileMapCs = new CRITICAL_SECTION();
		InitializeCriticalSection(g_FileMapCs);

		printf("[+] Starting AntiCheat Thread...\n");

		acHandle = (HANDLE)_beginthread(HandleAntiCheat, 0, 0);



	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{

		if (g_FileMapCs) {
			DeleteCriticalSection(g_FileMapCs);
			// delete g_FileMapCs; 
		}

		atexit(OnProcessExit);

		if (acHandle)
		{
			_CloseHandle ACCloseHandle = (_CloseHandle)GetProcAddress(GetModuleHandleA(kernel32), CloseHandle_func);
			if (ACCloseHandle)
				ACCloseHandle(acHandle);
		}
		if (ac)
			delete ac;

	}

	return TRUE;



}

void CEngine::ShowMessage(const char* STRINGEX)
{
	if (!MsgBox_Fn || !STRINGEX) return;

	static char msgBuffer[1024];
	strncpy_s(msgBuffer, sizeof(msgBuffer), STRINGEX, _TRUNCATE);
	GetInstance()->MsgBox_Fn(INGAME_MSG_PUSH_3, INGAME_MSG_PUSH_2_CRASH, INGAME_MSG_PUSH_1, (int)msgBuffer, INGAME_MSG_PUSH_1);
	Sleep(75000);
	std::exit((int)OnProcessExit);
}

void CEngine::ShowNormalMessage(const char* STRINGEX)
{
	if (!MsgBox_Fn || !STRINGEX) return;

	static char normalMsgBuffer[1024];

	strncpy_s(normalMsgBuffer, sizeof(normalMsgBuffer), STRINGEX, _TRUNCATE);

	GetInstance()->MsgBox_Fn(INGAME_MSG_PUSH_3, INGAME_MSG_PUSH_2, INGAME_MSG_PUSH_1, (int)normalMsgBuffer, INGAME_MSG_PUSH_1);
	return;
}

void CEngine::ChatMessage(const char* msg)
{
	int inGame = *reinterpret_cast<int*>(CShell + 0x016B3F58 + 0x7C);
	if (inGame == 1)
	{

		void* pChatInstance = GetChatInstance();

		if (pChatInstance && oSendChatPacket)
		{
			oSendChatPacket(pChatInstance, msg);
		}

	}
}