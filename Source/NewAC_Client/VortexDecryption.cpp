#include "SAC.h"
#include "VortexDecryption.h"

CRITICAL_SECTION* g_FileMapCs = nullptr;
std::map<HANDLE, OpenFileInfo>* g_OpenFiles = nullptr;
ENGINE_ENCRYPTION EncryptionEngine = ENGINE_ENCRYPTION();

bool VortexEncryption(DWORD_PTR hCrossfire);

// =============================================================
// Helpers
// =============================================================

thread_local int g_CrashStep = 0;

int LogCrash(const char* func, int step, DWORD excCode) {
#if ENABLE_VORTEX_LOG == 1
	printf("[CRASH] Exception in %s at STEP: %d | Code: 0x%X", func, step, excCode);
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}

bool IsTargetFile(const char* fileName) {
	if (!fileName) return false;
	size_t len = strlen(fileName);
	if (len < 4) return false;
	const char* ext = fileName + len - 4;

	if (_stricmp(ext, _xor(".rez").c_str()) == 0) {
		return true;
	}
	return false;
}

// =============================================================
// Hooks (Clean & Simple)
// =============================================================


BOOL Internal_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped, int* stepRef) {
	*stepRef = 1; // Start

	if (!g_FileMapCs || !g_OpenFiles) return EncryptionEngine.oReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	bool isEncrypted = false;
	uint32_t headerSz = 0;

	*stepRef = 2; // Map Lookup
	{
		CSLock lock(g_FileMapCs);
		auto it = g_OpenFiles->find(hFile);
		if (it != g_OpenFiles->end()) {

			if (!it->second.isChecked) {
				*stepRef = 3; // Check Header
				it->second.isChecked = true;

				DWORD curPos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
				DWORD fileSize = GetFileSize(hFile, NULL);

				uint32_t hLen = (uint32_t)HEADER_SIG.length();
				uint32_t fLen = (uint32_t)FOOTER_SIG.length();

				if (fileSize > hLen + fLen + 4) {
					*stepRef = 4; // Read Header
					SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
					std::vector<char> hBuf(hLen);
					DWORD read = 0;
					EncryptionEngine.oReadFile(hFile, hBuf.data(), hLen, &read, NULL);

					if (memcmp(hBuf.data(), HEADER_SIG.c_str(), hLen) == 0) {
						*stepRef = 5; // Read Footer
						SetFilePointer(hFile, fileSize - fLen, NULL, FILE_BEGIN);
						std::vector<char> fBuf(fLen);
						EncryptionEngine.oReadFile(hFile, fBuf.data(), fLen, &read, NULL);

						if (memcmp(fBuf.data(), FOOTER_SIG.c_str(), fLen) == 0) {
							*stepRef = 6; // Read Name
							SetFilePointer(hFile, hLen, NULL, FILE_BEGIN);
							uint32_t nameLen = 0;
							EncryptionEngine.oReadFile(hFile, &nameLen, 4, &read, NULL);

							if (nameLen > 0 && nameLen < 260) {
								it->second.isEncrypted = true;
								it->second.headerSize = hLen + 4 + nameLen;
#if ENABLE_VORTEX_LOG == 1
								printf("[ReadFile] ENCRYPTED DETECTED: %s | HeaderSz: %d", it->second.fileName.c_str(), it->second.headerSize);
#endif
							}
						}
					}
				}
				*stepRef = 7; // Restore
				SetFilePointer(hFile, curPos, NULL, FILE_BEGIN);
			}

			if (it->second.isEncrypted) {
				isEncrypted = true;
				headerSz = it->second.headerSize;
			}
		}
	}

	*stepRef = 8; // Native Read Check
	if (!isEncrypted) {
		return EncryptionEngine.oReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}

	*stepRef = 9; // Virtual Seek
	DWORD virtualPos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
	DWORD physicalPos = virtualPos + headerSz;

	*stepRef = 10; // Physical Seek
	if (SetFilePointer(hFile, physicalPos, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
#if ENABLE_VORTEX_LOG == 1
		printf("[ReadFile] ERROR: SetFilePointer Failed!");
#endif
	}

	*stepRef = 11; // Actual Read
	BOOL result = EncryptionEngine.oReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	*stepRef = 12; // Restore Virtual
	DWORD bytesRead = (lpNumberOfBytesRead) ? *lpNumberOfBytesRead : nNumberOfBytesToRead;
	SetFilePointer(hFile, virtualPos + bytesRead, NULL, FILE_BEGIN);

	*stepRef = 13; // Decrypt
	if (result && bytesRead > 0 && lpBuffer) {
		BYTE* pData = (BYTE*)lpBuffer;
		size_t keyLen = SECRET_KEY.length();

		for (DWORD i = 0; i < bytesRead; i++) {
			pData[i] ^= (BYTE)SECRET_KEY[(virtualPos + i) % keyLen];
		}
	}

	return result;
}

void Internal_CloseHandleLogic(HANDLE hObject) {
	if (!g_FileMapCs || !g_OpenFiles) return;

	CSLock lock(g_FileMapCs);
	if (g_OpenFiles->count(hObject)) {
#if ENABLE_VORTEX_LOG == 1
		// printf("[CloseHandle] Closing tracked handle: %p", hObject);
#endif
		g_OpenFiles->erase(hObject);
	}
}

DWORD GetModuleSize(HMODULE hModule) {
	if (!hModule) return 0;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return 0; // تأكد إنه PE
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return 0;
	return pNtHeaders->OptionalHeader.SizeOfImage;
}

// =============================================================
// Hooks (Wrappers with SEH)
// =============================================================

HANDLE WINAPI Hooked_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSec, DWORD dwDisp, DWORD dwFlags, HANDLE hTemplate) {




	HANDLE hFile = EncryptionEngine.oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSec, dwDisp, dwFlags, hTemplate);

	if (hFile != INVALID_HANDLE_VALUE && IsTargetFile(lpFileName)) {

		void* pRetAddr = _ReturnAddress();

		if (g_FileMapCs && g_OpenFiles) {
			CSLock lock(g_FileMapCs);

			OpenFileInfo info;
			info.isChecked = false;
			info.isEncrypted = false;
			info.headerSize = 0;
			info.fileName = lpFileName;

			(*g_OpenFiles)[hFile] = info;

#if ENABLE_VORTEX_LOG == 1
			printf("[CreateFile] Tracking file: %s (Handle: %p)", lpFileName, hFile);
#endif
		}

	}
	return hFile;
}



BOOL WINAPI Hooked_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	int step = 0;
	__try {
		return Internal_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped, &step);
	}
	__except (LogCrash(_xor("Hooked_ReadFile").c_str(), step, GetExceptionCode())) {
		return FALSE;
	}
}

BOOL WINAPI Hooked_CloseHandle(HANDLE hObject) {
	__try {
		Internal_CloseHandleLogic(hObject);
	}
	__except (LogCrash(_xor("Hooked_CloseHandle").c_str(), 0, GetExceptionCode())) {}

	return EncryptionEngine.oCloseHandle(hObject);
}

bool VortexEncryption(DWORD_PTR hCrossfire) {
	VM_HEAVY_START

#if ENABLE_VORTEX_LOG == 1
		printf("[Vortex] Installing Win32 Hooks...");
#endif

	HMODULE hKernel = GetModuleHandleA(_xor("kernel32.dll").c_str());
	if (!hKernel) {
#if ENABLE_VORTEX_LOG == 1
		printf("[Vortex] ERROR: Kernel32 not found!");
#endif
		return false;
	}

	EncryptionEngine.oCreateFileA = (ENGINE_ENCRYPTION::CreateFileAFn)GetProcAddress(hKernel, _xor("CreateFileA").c_str());
	EncryptionEngine.oReadFile = (ENGINE_ENCRYPTION::ReadFileFn)GetProcAddress(hKernel, _xor("ReadFile").c_str());
	EncryptionEngine.oCloseHandle = (ENGINE_ENCRYPTION::CloseHandleFn)GetProcAddress(hKernel, _xor("CloseHandle").c_str());

#if ENABLE_VORTEX_LOG == 1
	printf("[Vortex] Addresses: CreateFileA=%p, ReadFile=%p, CloseHandle=%p", EncryptionEngine.oCreateFileA, EncryptionEngine.oReadFile, EncryptionEngine.oCloseHandle);
#endif

	if (EncryptionEngine.oCreateFileA && EncryptionEngine.oReadFile && EncryptionEngine.oCloseHandle) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)EncryptionEngine.oCreateFileA, Hooked_CreateFileA);
		DetourAttach(&(PVOID&)EncryptionEngine.oReadFile, Hooked_ReadFile);
		DetourAttach(&(PVOID&)EncryptionEngine.oCloseHandle, Hooked_CloseHandle);
		LONG err = DetourTransactionCommit();

#if ENABLE_VORTEX_LOG == 1
		printf("[Vortex] Hook Result: %d", err);
#endif
		return (err == NO_ERROR);
	}



#if ENABLE_VORTEX_LOG == 1
	printf("[Vortex] ERROR: Failed to get proc addresses.");
#endif
	return false;

	VM_HEAVY_END
		return false;
}