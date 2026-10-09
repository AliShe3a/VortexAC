#include <windows.h>
#include <string>
#include "VortexHooks.h"

// =============================================================
// 1. الثوابت والـ Offsets
// =============================================================

#define RVA_CLICK_LOGIN_BUTTON      0x17B520  
#define RVA_GET_UI_ITEM             0x850CC0  
#define RVA_GAMEFLOW_LOGIN_INSTANCE 0x1CA4B00 

#define OFFSET_TEXT_BUFFER_1    890   // 0x37A
#define OFFSET_TEXT_BUFFER_2    380   // 0x17C
#define OFFSET_BUFFER_FLAG      72092 // 0x1199C

// =============================================================
// 2. Definitions
// =============================================================

struct LoginCredentials {
	char username[32];
	char password[32];
};

typedef void(__thiscall* tClickLoginButton)(void* pThis);
typedef void* (__thiscall* tGetUIItemByCtrlName)(void* pMgr, const char* szCtrlName);

class CUIItem {
public:
	void SetTextInMemory(const char* text) {
		// Safe memory writing with exception handling
		__try {
			unsigned char* pBase = (unsigned char*)this;
			bool useSecondaryBuffer = (pBase[OFFSET_BUFFER_FLAG] == 0);
			unsigned char* pTargetBuffer = useSecondaryBuffer ? (pBase + OFFSET_TEXT_BUFFER_2) : (pBase + OFFSET_TEXT_BUFFER_1);

			memset(pTargetBuffer, 0, 32);
			strncpy((char*)pTargetBuffer, text, 20);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			// Silently handle memory access violations
		}
	}
};

// =============================================================
// 3. Isolated SEH Logic (Safe Zone)
// =============================================================

// This function ONLY contains primitive types, safe for __try/__except
bool AttemptLoginSafe(void* pInstance, uintptr_t fnGetUIItemAddr, uintptr_t fnClickLoginAddr, const char* user, const char* pass)
{
	auto fnGetUIItem = (tGetUIItemByCtrlName)fnGetUIItemAddr;
	auto fnClickLogin = (tClickLoginButton)fnClickLoginAddr;

	__try
	{
		// 1. Check if Instance Pointer is valid
		if (IsBadReadPtr(pInstance, 4)) return false;

		// Manager is usually at offset 8
		void* pUIItemMgr = (void*)((DWORD)pInstance + 8);
		if (IsBadReadPtr(pUIItemMgr, 4)) return false;

		// 2. Try to get UI Items (The risky part)
		CUIItem* pEditID = (CUIItem*)fnGetUIItem(pUIItemMgr, "EditLoginID");
		if (!pEditID) return false;

		CUIItem* pEditPW = (CUIItem*)fnGetUIItem(pUIItemMgr, "EditLoginPass");
		if (!pEditPW) return false;

		// 3. Inject Data
		pEditID->SetTextInMemory(user);
		pEditPW->SetTextInMemory(pass);

		//Sleep(500);

		// 4. Click Login
		fnClickLogin(pInstance);

		return true; // Success
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Crash Blocked - Game not ready yet
		return false;
	}
}

// =============================================================
// 4. Worker Thread
// =============================================================

DWORD WINAPI AutoLoginWorker(LPVOID lpParam)
{
	LoginCredentials* creds = (LoginCredentials*)lpParam;
	std::string user = creds->username;
	std::string pass = creds->password;
	delete creds;

	uintptr_t dwCShellBase = 0;

	// 1. Wait for CShell.dll
	while ((dwCShellBase = (uintptr_t)GetModuleHandleA("CShell.dll")) == 0) {
		Sleep(100);
	}

	// Prepare raw addresses
	uintptr_t fnClickLoginAddr = dwCShellBase + RVA_CLICK_LOGIN_BUTTON;
	uintptr_t fnGetUIItemAddr = dwCShellBase + RVA_GET_UI_ITEM;
	void* pInstance = (void*)(dwCShellBase + RVA_GAMEFLOW_LOGIN_INSTANCE);

	// 2. The Loop
	while (true)
	{
		// Check every 500ms (balanced between performance and response time)
		Sleep(500);

		// Optional kill switch
		if (GetAsyncKeyState(VK_END) & 1) break;

		// Try to login safely
		if (AttemptLoginSafe(pInstance, fnGetUIItemAddr, fnClickLoginAddr, user.c_str(), pass.c_str()))
		{
			break; // Mission accomplished
		}
	}

	return 0;
}

// =============================================================
// 5. Execution
// =============================================================
bool ExecuteAutoLogin()
{
	HANDLE hMapFile = NULL;

	for (int i = 0; i < 5; i++) {
		hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "CF_LOGIN_DATA_SHARED");
		if (hMapFile != NULL) break;
		Sleep(1000);
	}

	if (hMapFile == NULL) return false;

	char* pBuf = (char*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 1024);
	if (pBuf == NULL) {
		CloseHandle(hMapFile);
		return false;
	}

	std::string rawData(pBuf);
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	if (rawData.empty() || rawData.find(':') == std::string::npos) return false;

	size_t separatorPos = rawData.find(':');
	std::string user = rawData.substr(0, separatorPos);
	std::string pass = rawData.substr(separatorPos + 1);

	if (user.length() < 1 || user.length() > 31 || pass.length() < 1 || pass.length() > 31) {
		return false;
	}

	LoginCredentials* creds = new LoginCredentials();
	memset(creds, 0, sizeof(LoginCredentials));

	strncpy_s(creds->username, user.c_str(), _TRUNCATE);
	strncpy_s(creds->password, pass.c_str(), _TRUNCATE);

	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoLoginWorker, creds, 0, NULL);
	if (hThread == NULL) {
		delete creds;
		return false;
	}

	CloseHandle(hThread);
	return true;
}

