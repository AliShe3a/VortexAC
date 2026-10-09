#include "SAC.h"
#include "stdafx.h"
#include "inject.h"
#include "ErrorCodes.h"
#include <thread>
#include "MemoryAddr.h"
#include <algorithm>
#include <ranges>
#include <Softpub.h> 
#include <shlwapi.h>
#include <chrono>
#include <wintrust.h>
#include <wincrypt.h>
#include <Psapi.h>
#include "modedhead.h"
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include "VortexDecryption.h"
#include "VortexHooks.h"
#pragma comment(lib, "wintrust.lib")
const BYTE EncryptionKey[] = { 0x87, 0x1A, 0x3C, 0x47, 0x9F, 0xE5, 0x56, 0x52, 0xEB, 0xCF, 0x71, 0x20, 0xB3, 0xAE, 0x92, 0xDE, 0x2B, 0x4E, 0xBD, 0x20, 0xC6, 0x5C, 0x13, 0xE2, 0x74, 0x0F, 0xAF, 0x6A, 0x0F, 0xA1, 0x6E, 0x28, 0x1D, 0x01, 0x19, 0x58, 0x57, 0x55, 0x80, 0xDC, 0xC8, 0x8E, 0xC6, 0xA9, 0xD5, 0x88, 0x84, 0x0C, 0x29, 0xDD, 0x25, 0x1B, 0xC4, 0xA8, 0xE7, 0x2B, 0x06, 0x59, 0xD1, 0x90, 0x35, 0xEA, 0x43, 0x4A, 0x61, 0x48, 0x03, 0xB1, 0x95, 0x3A, 0x86, 0xD5, 0xCC, 0xDF, 0x56, 0x37, 0x94, 0x68, 0x0C, 0xEB, 0x89, 0xA3, 0x41, 0x31, 0x7B, 0x52, 0x63, 0x74, 0x18, 0xEC, 0x51, 0xFD, 0x82, 0xEF, 0xA6, 0xDD, 0x4B, 0x7C, 0x48, 0x8D, 0x85, 0xD1, 0x76, 0xED, 0x56, 0x4F, 0x4D, 0xE3, 0x51, 0xB0, 0x95, 0x9D, 0x4C, 0xE5, 0x06, 0x91, 0xA8, 0x11, 0xCD, 0x0E, 0xE4, 0x8D, 0x99, 0xA7, 0xE7, 0x88, 0x52, 0xDC, 0x39, 0x78, 0xEB, 0x18, 0x20, 0x7E, 0x66, 0xB6, 0x2C, 0x52, 0xDA, 0x2B, 0x05, 0x7F, 0x22, 0x67, 0x56, 0xB3, 0xB1, 0x9C, 0xE8, 0x14, 0x34, 0x39, 0xAD, 0x39, 0xA7, 0xF6, 0x38, 0xCE, 0x06, 0xCC, 0xFC, 0x70, 0xA9, 0xD7, 0xC3, 0x7D, 0x7D, 0xCC, 0x7F, 0x4F, 0x65, 0x56, 0x36, 0xA2, 0x09, 0x64, 0x6B, 0x32, 0x50, 0x34, 0xD6, 0xB5, 0xF2, 0x25, 0x44, 0xB1, 0x6A, 0xE5, 0xD5, 0x6F, 0x7E, 0x5D, 0x4C, 0x0D, 0x2E, 0x93, 0xD4, 0x4A, 0xA2, 0xD8, 0x35, 0xE3, 0x1D, 0x99, 0xAE, 0xDC, 0x03, 0x0B, 0x2F, 0x43, 0x8A, 0x06, 0x7D, 0xD0, 0xF5, 0x46, 0x7D, 0x84, 0x92, 0x12, 0x12, 0x96, 0xF5, 0x10, 0xCF, 0x07, 0x09, 0x28, 0x73, 0x74, 0x8E, 0x21, 0x8E, 0x76, 0xFE, 0xF1, 0x98, 0x58, 0x75, 0x66, 0xAE, 0x5F, 0xB6, 0x92, 0xE3, 0x77, 0xDE, 0x7B, 0x17, 0xE6, 0x16, 0x50, 0xB6, 0x90, 0xC6, 0x9E };
She3aAC* She3aAC::Instance = NULL;
PlayerInfo* PlayerInfo::Instance = NULL;
extern "C" IMAGE_DOS_HEADER __ImageBase;
OwnDllInfo g_OwnDllInfo;
namespace fs = std::filesystem;

#pragma pack(push, 1)
struct ZIP_LOCAL_HEADER {
	uint32_t signature = 0x04034b50; // PK\003\004
	uint16_t version = 10;
	uint16_t flags = 0;
	uint16_t compression = 0; // 0 = Store (No Compression)
	uint16_t modTime = 0;
	uint16_t modDate = 0;
	uint32_t crc32 = 0;
	uint32_t compressedSize = 0;
	uint32_t uncompressedSize = 0;
	uint16_t fileNameLength = 0;
	uint16_t extraFieldLength = 0;
};

struct ZIP_CENTRAL_DIR {
	uint32_t signature = 0x02014b50; // PK\001\002
	uint16_t versionMadeBy = 20;
	uint16_t versionNeeded = 10;
	uint16_t flags = 0;
	uint16_t compression = 0;
	uint16_t modTime = 0;
	uint16_t modDate = 0;
	uint32_t crc32 = 0;
	uint32_t compressedSize = 0;
	uint32_t uncompressedSize = 0;
	uint16_t fileNameLength = 0;
	uint16_t extraFieldLength = 0;
	uint16_t fileCommentLength = 0;
	uint16_t diskNumberStart = 0;
	uint16_t internalAttributes = 0;
	uint32_t externalAttributes = 0;
	uint32_t localHeaderOffset = 0;
};

struct ZIP_EOCD {
	uint32_t signature = 0x06054b50; // PK\005\006
	uint16_t diskNumber = 0;
	uint16_t startDiskNumber = 0;
	uint16_t numberCentralDirectoryEntriesOnDisk = 0;
	uint16_t totalNumberOfCentralDirectoryEntries = 0;
	uint32_t centralDirectorySize = 0;
	uint32_t centralDirectoryOffset = 0;
	uint16_t commentLength = 0;
};
#pragma pack(pop)

void She3aAC::EncryptDatas(BYTE* datas, size_t len)
{

	VIRTUALIZER_START

		for (int i = 0; i < len; i++)
		{
			datas[i] ^= EncryptionKey[i % 256];
			datas[i] = ~datas[i];
			datas[i] ^= EncryptionKey[255 - i % 256];
		}


	VIRTUALIZER_END

}

void She3aAC::DecryptDatas(BYTE* datas, size_t len)
{

	VIRTUALIZER_START

		for (int i = 0; i < len; i++)
		{
			datas[i] ^= EncryptionKey[255 - i % 256];
			datas[i] = ~datas[i];
			datas[i] ^= EncryptionKey[i % 256];
		}

	VIRTUALIZER_END

}

// Need Xor Enc

static std::unordered_set<std::string> WhitelistedSRLs = {
_xor("D7 34 3E 09 83 0D 2E 06 A0 64 83 71 2E CF E9 0D").c_str(),  // discord
_xor("37 AB 4D C5 EE DE E8 DE 91 C1 B8 83 06 6A 41 0D").c_str(), // obs
_xor("D6 5A 9D 70 88 82 4F 3E E8 21 5E 08").c_str(), // msi
_xor("CE 60 D9 2E 95 CD 7A 48 02 B1 17 2F 0D 87 32 05").c_str(), // tiktok

};

static std::unordered_set<std::string> WhitelistedPublishers = {
	_xor("discord inc.").c_str(),         
	_xor("obs project, llc").c_str(),     
	_xor("micro-star international co., ltd.").c_str(),
	_xor("tiktok pte. ltd.").c_str(),
};

static std::unordered_set<std::string> WhitelistedModuleNames = {
	_xor("discordhook.dll").c_str(),      
	_xor("graphics-hook32.dll").c_str(),
	_xor("rtsshooks.dll").c_str(),        
	_xor("game_detour_32.dll").c_str(),
};

DWORD She3aAC::CalculateCRC32(const BYTE* data, size_t size)
{
	DWORD crc = 0xFFFFFFFF;
	for (size_t i = 0; i < size; ++i)
	{
		crc ^= data[i];
		for (int j = 0; j < 8; ++j)
			crc = (crc >> 1) ^ (0xEDB88320 & (-(int)(crc & 1)));
	}
	return ~crc;
}

std::string She3aAC::GetCertSerialCode(const char* filePath)
{
	std::string serialStr = _xor("Unknown").c_str();

	std::wstring wFilePath(filePath, filePath + strlen(filePath));

	WINTRUST_FILE_INFO fileInfo = {};
	fileInfo.cbStruct = sizeof(fileInfo);
	fileInfo.pcwszFilePath = wFilePath.c_str();
	fileInfo.hFile = NULL;

	WINTRUST_DATA trustData = {};
	trustData.cbStruct = sizeof(trustData);
	trustData.dwUIChoice = WTD_UI_NONE;
	trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	trustData.dwUnionChoice = WTD_CHOICE_FILE;
	trustData.pFile = &fileInfo;
	trustData.dwStateAction = WTD_STATEACTION_VERIFY;
	trustData.dwProvFlags = WTD_SAFER_FLAG;
	trustData.dwUIContext = 0;

	GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

	LONG status = WinVerifyTrust(NULL, &policyGUID, &trustData);

	if (status != ERROR_SUCCESS)
	{
		// Not signed or verification failed
		return serialStr;
	}

	// Access the state data (signed cert info)
	CRYPT_PROVIDER_DATA* provData = WTHelperProvDataFromStateData(trustData.hWVTStateData);
	if (!provData)
	{
		trustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &policyGUID, &trustData);
		return serialStr;
	}

	CRYPT_PROVIDER_SGNR* provSigner = WTHelperGetProvSignerFromChain(provData, 0, FALSE, 0);
	if (!provSigner || provSigner->csCertChain == 0)
	{
		trustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &policyGUID, &trustData);
		return serialStr;
	}

	CRYPT_PROVIDER_CERT* provCert = WTHelperGetProvCertFromChain(provSigner, 0);
	if (!provCert || !provCert->pCert)
	{
		trustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &policyGUID, &trustData);
		return serialStr;
	}

	// Extract the serial number
	CRYPT_INTEGER_BLOB serialBlob = provCert->pCert->pCertInfo->SerialNumber;

	std::ostringstream oss;
	for (DWORD i = 0; i < serialBlob.cbData; ++i)
	{
		oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)serialBlob.pbData[i];
		if (i + 1 < serialBlob.cbData)
			oss << " ";
	}
	serialStr = oss.str();


	{
		trustData.dwStateAction = WTD_STATEACTION_CLOSE;
		WinVerifyTrust(NULL, &policyGUID, &trustData);
		return serialStr;
	}
}

void She3aAC::InitOwnDllInfo()
{

	HMODULE hModule = (HMODULE)&__ImageBase;
	if (hModule == NULL) {
		//	printf("[!] Error: hModule is NULL.\n");
		return;
	}

	try {
		PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
		if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
			PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
			if (ntHeaders->Signature == IMAGE_NT_SIGNATURE) {

				g_OwnDllInfo.moduleBase = (DWORD)hModule;
				g_OwnDllInfo.moduleSize = ntHeaders->OptionalHeader.SizeOfImage;

			
			}
		}
	}
	catch (...) {
		//	printf("[!] Step 2 Error: Failed to parse PE Header.\n");
	}


	char path[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(hModule, path, MAX_PATH))
	{
		g_OwnDllInfo.modulePath = path;
		g_OwnDllInfo.moduleName = PathFindFileNameA(path);
		//printf("[+] Step 3: Name = %s\n", g_OwnDllInfo.moduleName.c_str());
	}


	g_OwnDllInfo.crc = CalculateCRC32((BYTE*)hModule, 16);
	//printf("[+] Step 4: CRC32 = 0x%08X\n", g_OwnDllInfo.crc);

	//printf("--- [Debug: Initialization Finished] ---\n\n");
}

std::string She3aAC::GetModulePublisherNameSafe(const char* modulePath)
{
	std::string publisher = _xor("Unknown").c_str();

	std::wstring wPath(modulePath, modulePath + strlen(modulePath));

	// Prepare WinTrust structures
	WINTRUST_FILE_INFO fileInfo = {};
	fileInfo.cbStruct = sizeof(fileInfo);
	fileInfo.pcwszFilePath = wPath.c_str();
	fileInfo.hFile = NULL;
	fileInfo.pgKnownSubject = NULL;

	WINTRUST_DATA trustData = {};
	trustData.cbStruct = sizeof(trustData);
	trustData.dwUIChoice = WTD_UI_NONE;
	trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	trustData.dwUnionChoice = WTD_CHOICE_FILE;
	trustData.dwStateAction = WTD_STATEACTION_VERIFY;
	trustData.dwProvFlags = WTD_SAFER_FLAG;
	trustData.pFile = &fileInfo;

	GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

	LONG status = WinVerifyTrust(NULL, &policyGUID, &trustData);
	if (status != ERROR_SUCCESS)
	{
		//std::cout << "[ERROR] WinVerifyTrust failed. Code: " << std::hex << status << std::endl;
		return publisher;
	}

	// Get cert context from the state data
	CRYPT_PROVIDER_DATA* providerData = WTHelperProvDataFromStateData(trustData.hWVTStateData);
	if (!providerData)
	{
		//std::cout << "[ERROR] WTHelperProvDataFromStateData failed.\n";
		return publisher;
	}

	CRYPT_PROVIDER_SGNR* signer = WTHelperGetProvSignerFromChain(providerData, 0, FALSE, 0);
	if (!signer || signer->csCertChain == 0)
	{
		//std::cout << "[ERROR] No signer or certificate chain found.\n";
		return publisher;
	}

	// Use the first certificate (leaf)
	PCCERT_CONTEXT certContext = signer->pasCertChain[0].pCert;
	if (!certContext)
	{
		//std::cout << "[ERROR] Failed to get certificate context.\n";
		return publisher;
	}

	char nameBuffer[512] = {};
	if (CertGetNameStringA(certContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, nameBuffer, sizeof(nameBuffer)) > 1)
	{
		publisher = nameBuffer;
		//std::cout << "[LOG] Publisher: " << publisher << std::endl;
	}
	else
	{
		//std::cout << "[WARN] CertGetNameStringA failed.\n";
	}

	// Cleanup
	trustData.dwStateAction = WTD_STATEACTION_CLOSE;
	WinVerifyTrust(NULL, &policyGUID, &trustData);

	return publisher;
}

std::string ToLower(const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

bool She3aAC::IsModuleSigned(const char* modulePath)
{
	if (!modulePath || !PathFileExistsA(modulePath))
		return false;

	std::wstring widePath(modulePath, modulePath + strlen(modulePath));

	WINTRUST_FILE_INFO fileInfo = { sizeof(WINTRUST_FILE_INFO) };
	fileInfo.pcwszFilePath = widePath.c_str();

	WINTRUST_DATA trustData = { sizeof(WINTRUST_DATA) };
	trustData.dwUIChoice = WTD_UI_NONE;
	trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	trustData.dwUnionChoice = WTD_CHOICE_FILE;
	trustData.pFile = &fileInfo;
	trustData.dwStateAction = WTD_STATEACTION_VERIFY;
	trustData.dwProvFlags = WTD_REVOCATION_CHECK_NONE;

	GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

	return (WinVerifyTrust(NULL, &policyGUID, &trustData) == ERROR_SUCCESS);
}

HookInfo She3aAC::GetHookInfoFromAddress(DWORD AddrStart)
{
	HookInfo info = {};
	info.address = AddrStart;

	BYTE* pCode = (BYTE*)AddrStart;

	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery(pCode, &mbi, sizeof(mbi)))
		return info; // failed to get info

	HMODULE hMod = (HMODULE)mbi.AllocationBase;

	char modPath[MAX_PATH] = {};
	if (!GetModuleFileNameA(hMod, modPath, MAX_PATH))
		return info; // failed to get module path

	info.modulePath = modPath;
	info.moduleName = PathFindFileNameA(modPath);

	// Compute CRC on first 16 bytes at AddrStart
	info.crc = CalculateCRC32(pCode, 16);

	// Check if signed
	info.isSigned = IsModuleSigned(modPath);

	// Get publisher name only if signed
	if (info.isSigned)
	{
		info.publisherName = GetModulePublisherNameSafe(modPath);
		info.SerialCode = GetCertSerialCode(modPath);
	}
	else
	{
		info.publisherName = _xor("N/A").c_str();
		info.SerialCode = _xor("N/A").c_str();
	}

	return info;
}

bool She3aAC::IsHookInfoWhitelisted(const HookInfo& info)
{
	
	if (info.address >= g_OwnDllInfo.moduleBase &&
		info.address <= (g_OwnDllInfo.moduleBase + g_OwnDllInfo.moduleSize))
	{
		return true; // AC Module Hook - Allowed!
	}

	if (!info.moduleName.empty() && !g_OwnDllInfo.moduleName.empty() &&
		_stricmp(info.moduleName.c_str(), g_OwnDllInfo.moduleName.c_str()) == 0)
	{
		return true; // AC Module Hook - Allowed!
	}

	std::string modLower = ToLower(info.moduleName);
	if (WhitelistedModuleNames.find(modLower) != WhitelistedModuleNames.end())
	{
		std::string pubLower = ToLower(info.publisherName);
		if (WhitelistedPublishers.find(pubLower) != WhitelistedPublishers.end())
		{
			if (WhitelistedSRLs.find(info.SerialCode) != WhitelistedSRLs.end())
				return true;
		}
	}

	return false; // Not whitelisted by any criteria
}

bool She3aAC::IsTextSectionHookPresent(DWORD AddrStart)
{
	BYTE* pCode = (BYTE*)AddrStart;


	if (*pCode == 0xE9) // JMP rel32
	{
		int32_t relOffset = *(int32_t*)(pCode + 1);
		DWORD hookTarget = (DWORD)(pCode + 5 + relOffset);

		HookInfo info = GetHookInfoFromAddress(hookTarget);

		//printf("\n--- D3D MODULE HOOK ---\n");
		//printf("pCode       : 0x%08X\n", 0xE9);
		//printf("Address       : 0x%08X\n", info.address);
		//printf("Module        : %s\n", info.moduleName.c_str());
		//printf("isSigned      : %s\n", info.isSigned ? "true" : "false");
		//printf("mCRC          : 0x%08X\n", info.crc);
		//printf("Publisher Name: %s\n", info.publisherName.c_str());
		//printf("Serial Code: %s\n", info.SerialCode.c_str());
		//printf("Module Path   : %s\n", info.modulePath.c_str());

		if (IsHookInfoWhitelisted(info))
		{
			//printf("Hook is WHITELISTED (allowed)\n");
			return false; // Treat as NOT hooked, allowed overlay
		}
		else
		{

			//printf("Hook is NOT whitelisted! (potential cheat)\n");
			return true; // Hook detected and NOT trusted
		}
	}
	else if (*pCode == 0xE8) // CALL rel32 (similar parsing)
	{
		int32_t relOffset = *(int32_t*)(pCode + 1);
		DWORD callTarget = (DWORD)(pCode + 5 + relOffset);
		HookInfo info = GetHookInfoFromAddress(callTarget);


		//printf("\n--- D3D MODULE HOOK ---\n");
		//printf("pCode       : 0x%08X\n", 0xE8);
		//printf("Address       : 0x%08X\n", info.address);
		//printf("Module        : %s\n", info.moduleName.c_str());
		//printf("isSigned      : %s\n", info.isSigned ? "true" : "false");
		//printf("mCRC          : 0x%08X\n", info.crc);
		//printf("Publisher Name: %s\n", info.publisherName.c_str());
		//printf("Serial Code: %s\n", info.SerialCode.c_str());
		//printf("Module Path   : %s\n", info.modulePath.c_str());

		if (IsHookInfoWhitelisted(info))
		{
			//printf("Hook is WHITELISTED (allowed)\n");
			return false; // Treat as NOT hooked, allowed overlay
		}
		else
		{

			//printf("Hook is NOT whitelisted! (potential cheat)\n");
			return true; // Hook detected and NOT trusted
		}
	}
	else if (*pCode == 0xFF) // JMP/CALL [reg/mem] - more complex
	{

		HookInfo info = GetHookInfoFromAddress(AddrStart);



		if (IsHookInfoWhitelisted(info))
		{
			//printf("Hook is WHITELISTED (allowed)\n");
			return false; // Treat as NOT hooked, allowed overlay
		}
		else
		{

			//printf("Hook is NOT whitelisted! (potential cheat)\n");
			return true; // Hook detected and NOT trusted
		}
	}

	return false;
}

std::string GetModuleNameFromAddress(DWORD address) {
	HMODULE hModule = NULL;
	char buffer[MAX_PATH];

	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)address, &hModule)) {

		if (GetModuleFileNameA(hModule, buffer, sizeof(buffer))) {
			std::string path = buffer;
			return path.substr(path.find_last_of("\\/") + 1);
		}
	}
	return _xor("Unknown Module").c_str();
}

bool She3aAC::ValidateAddress(DWORD Addr, const char* ModuleName)
{
	DWORD callerAddress = (DWORD)_ReturnAddress();
	DWORD moduleStart = g_OwnDllInfo.moduleBase;
	DWORD moduleEnd = g_OwnDllInfo.moduleBase + g_OwnDllInfo.moduleSize;

	if (callerAddress < moduleStart || callerAddress > moduleEnd)
	{

		std::string moduleName = cPlayer->cEngine->GetModuleNameFromAddress(callerAddress);

		long long offset = 0;
		if (callerAddress < moduleStart)
			offset = (long long)callerAddress - (long long)moduleStart;
		else
			offset = (long long)callerAddress - (long long)moduleEnd;

		//char debugMsg[1024];
		//sprintf_s(debugMsg,
		//	"\n--- [Security Alert: Unauthorized Caller] ---\n"
		//	"Actual Caller Address: 0x%08X\n"
		//	"Expected Range:        [0x%08X - 0x%08X]\n"
		//	"Distance from Boundary:%lld bytes (%s)\n"
		//	"Caller Module Name:    %s\n"
		//	"---------------------------------------------\n",
		//	callerAddress,
		//	moduleStart,
		//	moduleEnd,
		//	offset,
		//	(callerAddress < moduleStart ? "Below Start" : "Above End"),
		//	moduleName.c_str());

		//printf("%s", debugMsg);
		//OutputDebugStringA(debugMsg);


		__asm {
			cli            
			mov esp, 0      
			jmp esp        
		}

		TerminateProcess(GetCurrentProcess(), 0xC0000005);
	}
	// --------------------------------------------------

	if (Addr == 0) return false;

	HMODULE hMod = GetModuleHandleA(ModuleName);
	if (!hMod) return false;

	MODULEINFO mi;
	GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
	DWORD start = (DWORD)mi.lpBaseOfDll;
	DWORD end = start + mi.SizeOfImage;

	if (Addr < start || Addr > end)
	{
		HookInfo info = GetHookInfoFromAddress(Addr);

		if (IsHookInfoWhitelisted(info)) return false;

		return true;
	}

	if (IsTextSectionHookPresent(Addr))
	{
		return true; // True = Hook Detected!
	}

	return false;
}

FARPROC She3aAC::ACGetProcAddress(HMODULE module, const char* proc_name)
{
	char* modb = (char*)module;

	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)modb;
	IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)(modb + dos_header->e_lfanew);
	IMAGE_OPTIONAL_HEADER* opt_header = &nt_headers->OptionalHeader;
	IMAGE_DATA_DIRECTORY* exp_entry = (IMAGE_DATA_DIRECTORY*)
		(&opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	IMAGE_EXPORT_DIRECTORY* exp_dir = (IMAGE_EXPORT_DIRECTORY*)(modb + exp_entry->VirtualAddress);
	void** func_table = (void**)(modb + exp_dir->AddressOfFunctions);
	WORD* ord_table = (WORD*)(modb + exp_dir->AddressOfNameOrdinals);
	char** name_table = (char**)(modb + exp_dir->AddressOfNames);
	FARPROC address = NULL;

	DWORD i;

	/* is ordinal? */
	if (((DWORD)proc_name >> 16) == 0) {
		WORD ordinal = LOWORD(proc_name);
		DWORD ord_base = exp_dir->Base;
		/* is valid ordinal? */
		if (ordinal < ord_base || ordinal > ord_base + exp_dir->NumberOfFunctions)
			return NULL;

		/* taking ordinal base into consideration */
		address = (FARPROC)(modb + (DWORD)func_table[ordinal - ord_base]);
	}
	else {
		/* import by name */
		for (i = 0; i < exp_dir->NumberOfNames; i++) {
			/* name table pointers are rvas */
			if (strcmp(proc_name, modb + (DWORD)name_table[i]) == 0)
				address = (FARPROC)(modb + (DWORD)func_table[ord_table[i]]);
		}
	}
	/* is forwarded? */
	if ((char*)address >= (char*)exp_dir &&
		(char*)address < (char*)exp_dir + exp_entry->Size) {
		char* dll_name, * func_name;
		HMODULE frwd_module;
		dll_name = _strdup((char*)address);
		if (!dll_name)
			return NULL;
		address = NULL;
		func_name = strchr(dll_name, '.');
		*func_name++ = 0;

		/* is already loaded? */
		frwd_module = GetModuleHandleA(dll_name);
		if (!frwd_module)
			frwd_module = LoadLibraryA(dll_name);

		if (frwd_module)
			address = ACGetProcAddress(frwd_module, func_name);

		free(dll_name);
	}
	return address;
}

bool She3aAC::PatternCmp(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

bool She3aAC::bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)   return 0;
	return (*szMask) == NULL;
}


DWORD She3aAC::FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, const char* szMask, int chosen)
{
	int count = 1;
	int custominfovalue = 0;

	for (DWORD i = 0; i < dwLen; i++)
		if (bCompare((BYTE*)(dwAddress + i), bMask, szMask))
			if (count++ == chosen)
				return (DWORD)(dwAddress + i);

	//				Log("\nIsAntiShakeScreen ID: %d, FPS %f",custominfovalue,*(float*)(CShell + OFF_CSHELL_FPS));
	return 0;
}


DWORD She3aAC::FindPatternVideo(char* module, const char* pattern, const char* mask)
{
	DWORD code_section_offset, code_section_size;
	DWORD mod = (DWORD)GetModuleHandleA(module);

	if (!GetCodeSectionInfo(mod, &code_section_offset, &code_section_size))
		return NULL;

	//Get length for our mask, this will allow us to loop through our array
	DWORD patternLength = (DWORD)strlen(mask);
	bool found = true;
	//Log("base: 0x%08X, code section offset: 0x%08X, code section size: %d, pattern size: %d",mod, code_section_offset, code_section_size, patternLength);
	for (DWORD i = 0; i < (code_section_size / 2) - patternLength - 0x10; i++)
	{
		found = true;
		for (DWORD j = 0; j < patternLength && found; j++)
		{
			//Log("\n Kernel32.dll Address: 0x%08X", mod + i);
			//if we have a ? in our mask then we have true by default,
			//or if the bytes match then we keep searching until finding it or not
			found = mask[j] == '?' || pattern[j] == *(char*)(mod + code_section_offset + i + j);
		}

		////found = true, our entire pattern was found
		if (found)
		{
			return mod + code_section_offset + i;
		}
	}
	return NULL;
}

std::string ws2s(const std::wstring& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	char* buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
	std::string r(buf);
	delete[] buf;
	return r;
}


bool She3aAC::IsXMouseDetected()
{
	HWND hWindow = FindWindow(XMouseButtonControl, NULL);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(NULL, XMouseButtonControl);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(XMouseButtonControl, XMouseButtonControl);
	if (hWindow != NULL)
		return true;

	return false;
}


bool She3aAC::IsUMTScriptDetected()
{
	HWND hWindow = FindWindow(NULL, UMTBeta);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(NULL, UMTBetaShort);
	if (hWindow != NULL)
		return true;

	return false;
}
bool She3aAC::IsHookedCheatCMDDetected()
{
	HWND hWindow = FindWindow(ConsoleWindowClass, CROSSFIRE);
	if (hWindow != NULL)
		return true;

	return false;
}


LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue)

{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}
	return nError;
}

char* She3aAC::GetRegistry(char* Path, std::wstring Key)
{
	HKEY hKey;
	char szName[254];
	std::wstring strValueOfBinDir;
	std::wstring strKeyDefaultValue;

	LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, Path, 0, KEY_READ, &hKey);

	GetStringRegKey(hKey, Key, strValueOfBinDir, _xor(L"INVALID").c_str());

	WideCharToMultiByte(CP_ACP, 0, strValueOfBinDir.c_str(), -1, szName, 254, 0, 0);
	return szName;
}

DWORD She3aAC::FindPatternVideoWindows7(char* module, const char* pattern, const char* mask)
{
	DWORD code_section_offset, code_section_size;
	DWORD mod = (DWORD)GetModuleHandleA(module);

	if (!GetCodeSectionInfo(mod, &code_section_offset, &code_section_size))
		return NULL;

	//Get length for our mask, this will allow us to loop through our array
	DWORD patternLength = (DWORD)strlen(mask);
	bool found = true;
	//Log("base: 0x%08X, code section offset: 0x%08X, code section size: %d, pattern size: %d",mod, code_section_offset, code_section_size, patternLength);
	for (DWORD i = 0; i < code_section_size - patternLength - 0x10; i++)
	{
		found = true;
		for (DWORD j = 0; j < patternLength && found; j++)
		{
			//Log("\n Kernel32.dll Address: 0x%08X", mod + i);
			//if we have a ? in our mask then we have true by default,
			//or if the bytes match then we keep searching until finding it or not
			found = mask[j] == '?' || pattern[j] == *(char*)(mod + i + j);
		}

		////found = true, our entire pattern was found
		if (found)
		{
			return mod + i;
		}
	}
	return NULL;
}

bool She3aAC::IsIllegalProcessDetected()
{
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32FirstW(SnapShot, &procEntry))
		return false;

	do
	{
		if (wcscmp(procEntry.szExeFile, _xor(L"cheatengine-x86_64.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"cheatengine-x86_64-SSE4-AVX2.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"cheatengine-i386.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"Cheat Engine.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"x96dbg.exe").c_str()) == 0)
		{
			return true;
		}		if (wcscmp(procEntry.szExeFile, _xor(L"x32dbg.exe").c_str()) == 0)
		{
			return true;
		}		if (wcscmp(procEntry.szExeFile, _xor(L"x32dbg-unsigned.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"ollydbg.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"httpdebugger.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"acmb.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"HTTPDebuggerUI.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"HTTPDebuggerSvc.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"perfmon.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"mmc.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"Keyran.exe").c_str()) == 0)
		{
			return true;
		}
		if (wcscmp(procEntry.szExeFile, _xor(L"SystemInformer.exe").c_str()) == 0)
		{
			return true;
		}

	} while (Process32NextW(SnapShot, &procEntry));

	HWND hWindow = FindWindow(CheatEngineSettings, NULL);
	if (hWindow != NULL)
		return true;
	hWindow = FindWindow(NULL, CheatEngineSettings);
	if (hWindow != NULL)
		return true;
	hWindow = FindWindow(KEYRAN_D1, NULL);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(KEYRAN_D2, NULL);
	if (hWindow != NULL)
		return true;
	hWindow = FindWindow(NULL, KEYRAN_D1);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(NULL, KEYRAN_D2);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(fengyue, NULL);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(NULL, Phant0m);
	if (hWindow != NULL)
		return true;

	DWORD KeyRanIst = GetFileAttributesW(KEYRAN_D3);
	if (KeyRanIst != INVALID_FILE_ATTRIBUTES && (KeyRanIst & FILE_ATTRIBUTE_DIRECTORY)) {
		return true;
	}

	WIN32_FIND_DATAW findFileData;
	HANDLE hFind = FindFirstFileW(KEYRAN_D4, &findFileData);

	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return true;
	}

	return false;
}

bool She3aAC::IsTestSigningEnabled() {
	SYSTEM_CODEINTEGRITY_INFORMATION sci = { 0 };
	ULONG dwcbSz = 0;
	sci.Length = sizeof(sci);
	DWORD status;
	if ((status = NtQuerySystemInformation(
		/*SystemCodeIntegrityInformation*/ 0x67,
		&sci,
		sizeof(sci),
		&dwcbSz)) >= 0 &&
		dwcbSz == sizeof(sci))
	{
		return !!(sci.CodeIntegrityOptions & /*CODEINTEGRITY_OPTION_TESTSIGN*/ 0x2);
		//Note that testsigning will play no role if bit CODEINTEGRITY_OPTION_ENABLED (or 0x1) is not set in sci.CodeIntegrityOptions
	}
	return true;
	return false;
}

bool She3aAC::IsProcessDetected()
{
	HWND hWindow = FindWindow(AutoHotkey, NULL);
	if (hWindow != NULL)
		return true;
	return false;
}

HANDLE She3aAC::GetProcessHandle(const char* process_name, DWORD dwAccess)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	WCHAR process_name_wide[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, process_name, -1, process_name_wide, MAX_PATH);

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return NULL;
	}

	do
	{
		if (wcscmp(pe32.szExeFile, process_name_wide) == 0)
		{
			CloseHandle(hProcessSnap);
			return OpenProcess(dwAccess, 0, pe32.th32ProcessID);
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return NULL;
}

bool She3aAC::IsBitDefenderDetected()
{
	HWND hWindow = FindWindow(NULL, BitdefenderSecurityCenterName);
	if (hWindow != NULL)
		return true;

	hWindow = FindWindow(BitdefenderSecurityCenterClass, NULL);
	if (hWindow != NULL)
		return true;
	HANDLE hProcessAgent = NULL;
	HANDLE hProcessRedLine = NULL;
	hProcessAgent = GetProcessHandle(BDAGENT_PROCESS, PROCESS_QUERY_INFORMATION);
	hProcessRedLine = GetProcessHandle(BDREDLINE_PROCESS, PROCESS_QUERY_INFORMATION);
	if (hProcessAgent == NULL || hProcessAgent == INVALID_HANDLE_VALUE || hProcessRedLine == NULL || hProcessRedLine == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		return true;
	}

	return false;
}

bool She3aAC::AreMultipleClientRunning() {
	const wchar_t* processName = _xor(L"crossfire.exe").c_str();
	int count = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return false;
	}
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(processEntry);
	if (Process32First(snapshot, &processEntry)) {
		do {
			if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
				count++;
				if (count > 1) {
					CloseHandle(snapshot);
					return true;
				}
			}
		} while (Process32Next(snapshot, &processEntry));
	}

	CloseHandle(snapshot);

	return false;
}

bool She3aAC::IsHGWorXignNotLoaded()
{
	HWND hWindow = FindWindow(NULL, _xor(L"HGWC").c_str());
	if (hWindow == NULL)
		return true;

	//hWindow = FindWindow(XIGNCODE_TRAY, XIGNCODE);
	//if (hWindow == NULL)
	//	return true;

	return false;
}

bool She3aAC::GetCodeSectionInfo(DWORD base_address, DWORD* offset, DWORD* size)
{
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)base_address;
	if (dos_header->e_magic != 0x5A4D) // MZ
		return false;
	PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD)base_address + dos_header->e_lfanew);
	if (nt_headers->Signature != 0x4550) // PE
		return false;
	if (offset)
		*offset = nt_headers->OptionalHeader.BaseOfCode;
	if (size)
		*size = nt_headers->OptionalHeader.SizeOfCode;
	return true;
}

bool She3aAC::IsDllDetected()
{

	DWORD FoundDllimaadp64acm = (DWORD)GetModuleHandleA(imaadp64acm);
	DWORD Foundvctictadll = (DWORD)GetModuleHandleA(vctictadll);
	//DWORD vehdebug = (DWORD)GetModuleHandleA(vehdebugdll);
	//DWORD DiscordHookZ = (DWORD)GetModuleHandleA(DiscordHookDll);
	//DWORD DiscordHookC = (DWORD)GetModuleHandleA(DiscordOverLayDll);

	if (FoundDllimaadp64acm || Foundvctictadll  /* || vehdebug || DiscordHookZ || DiscordHookC*/)
		return true;

	return false;
}

void* DetourFunction(BYTE* src, const BYTE* dst, const int len)
{
	BYTE* jmp = (BYTE*)malloc(len + 5);
	DWORD dwBack;

	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &dwBack);
	memcpy(jmp, src, len);
	jmp += len;
	jmp[0] = 0xE9;
	*(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;
	for (int i = 5; i < len; i++)
		src[i] = 0x90;
	VirtualProtect(src, len, dwBack, &dwBack);
	return (jmp - len);
}

bool She3aAC::IsGameDebugged()
{
#ifndef NDEBUG
	return false;
#else
	if (IsDebuggerPresent() == TRUE)
		return true;
	try
	{
		throw 20;
	}
	catch (...)
	{
		return false;
	}
	return true;
#endif
}

DWORD WINAPI She3aAC::ReportErrorThreadWorker(LPVOID lpParam)
{
	ThreadParams* params = (ThreadParams*)lpParam;
	std::string BanKey = params->BanKey;
	delete params;

	std::string fullMsg = _xor("VORTEX SECURITY\n\nIllegal Program or Action Detected.\nCode Number : ").c_str() + BanKey + _xor("\n\nGame will be closed.").c_str();


	if (BanKey == ANTICHEAT_NOT_INITIALIIZED || BanKey == START_BANKEY || BanKey == VERSION_BANKEY || BanKey == PROCCESS_DETECT_BANKEY || BanKey == DEBUG_DETECT_BANKEY || BanKey == DUMP_DETECT_BANKEY || BanKey == CLIENT_STEAL_BANKEY || BanKey == REZ_EDIT_BANKEY || BanKey == TESTSIGN_ENABLED)
	{
		Instance->cPlayer->cEngine->ShowMessage(fullMsg.c_str());
		//Instance->CloseGame(0); 
		return 0;
	}

	if (!Instance->cPlayer->GotUserData)
	{
		Instance->CloseGame(0);
		return 0;
	}

	ErrorData* data = new ErrorData();
	data->code = BanKey;
	data->msg = Instance->cPlayer->BanMsg;

	if (HandleSendErrorCode(data)) {
		cout << "[+] DETECTION REPORT SENT TO SERVER : " << data->code << endl;
	}
	else {
		cout << "[-] FAILED TO SEND DETECTION REPORT" << endl;
		Instance->cPlayer->cEngine->ShowMessage(fullMsg.c_str());
		//Instance->CloseGame(0);
		return 0;
	}

	
	DWORD startTime = GetTickCount();
	const DWORD ERR_TIMEOUT_MS = 15000; 

	while (GetTickCount() - startTime < ERR_TIMEOUT_MS)
	{
		if (Instance->IsRecvedErrorReportResult)
		{
			Instance->cPlayer->cEngine->ShowMessage(fullMsg.c_str());

			//Instance->CloseGame(0);
			return 0;
		}

		Sleep(200); 
	}


	Instance->cPlayer->cEngine->ShowMessage(fullMsg.c_str());
	//Instance->CloseGame(0);

	return 0;
}

void She3aAC::ReportError(std::string BanKey)
{

	if (WasFlaggedAsCheater) return;

	WasFlaggedAsCheater = true; 

	ThreadParams* params = new ThreadParams();
	params->BanKey = BanKey;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)She3aAC::ReportErrorThreadWorker, params, 0, NULL);
}

void She3aAC::CustomInfoBox(const char* fmt, ...)
{
	va_list va_alist;
	char buf[256] = { 0 };
	va_start(va_alist, fmt);
	_vsnprintf_s(buf + strlen(buf), sizeof(buf) - strlen(buf), 256, fmt, va_alist);
	va_end(va_alist);
	ACMessageBox(0, buf, She3aCF, MB_OK);
}

void She3aAC::CloseGame(int uExitCode)
{
	//CustomInfoBox("CloseGame: %08X", uExitCode);
	//ExitMessageBox(uExitCode);
	ACExitProcess(uExitCode);
}

bool She3aAC::DirectoryExists(const char* dir)
{
	if (_access(dir, 0) == 0) {
		struct stat status;
		stat(dir, &status);
		return (status.st_mode & S_IFDIR) != 0;
	}
	return false;
}

bool She3aAC::SendAuthRequest()
{
	Instance->cPlayer->cEngine->CaptureScreenshot();
	VIRTUALIZER_START

		SEND_AUTH_REQUEST authRequest;
	memset((BYTE*)&authRequest, 0, sizeof(SEND_AUTH_REQUEST));
	authRequest.PacketID = CS_AUTH_REQ;
	authRequest.USN = cPlayer->UserUSN + 21;
	strncpy(authRequest.LoginID, cPlayer->UserURN.c_str(), sizeof(authRequest.LoginID) - 1);
	strncpy(authRequest.UserIGN, cPlayer->UserIGN.c_str(), sizeof(authRequest.UserIGN) - 1);
	strncpy(authRequest.Password, cPlayer->UserPWD.c_str(), sizeof(authRequest.Password) - 1);
	strncpy(authRequest.UserIP, cPlayer->UserIP.c_str(), sizeof(authRequest.UserIP) - 1);
	strncpy(authRequest.UserName, cPlayer->PC_UNAME.c_str(), sizeof(authRequest.UserName) - 1);
	strncpy(authRequest.ComputerName, cPlayer->PC_CNAME.c_str(), sizeof(authRequest.ComputerName) - 1);
	strncpy(authRequest.DiscordID, cPlayer->UserDID.c_str(), sizeof(authRequest.DiscordID) - 1);
	strncpy(authRequest.HardwareUUID, cPlayer->UserUUID.c_str(), sizeof(authRequest.HardwareUUID) - 1);
	strncpy(authRequest.HardwareGUID, cPlayer->UserGUID.c_str(), sizeof(authRequest.HardwareGUID) - 1);
	authRequest.CurVersion = acVersion;

	VIRTUALIZER_END

		return SendPacketToServer((BYTE*)&authRequest, sizeof(SEND_AUTH_REQUEST));


}

void She3aAC::SendLogOut()
{
	if(Instance->cPlayer->GotUserData)
{

	AC_MSG_PACKET logout;
	memset((BYTE*)&logout, 0, sizeof(AC_MSG_PACKET));
	logout.PacketID = CS_LOGOUT_REQ;
	logout.USN = Instance->cPlayer->UserUSN + 21;
	Instance->SendPacketToServer((BYTE*)&logout, sizeof(AC_MSG_PACKET));

}
	return;
}

bool She3aAC::SendAcHeartbeat()
{

	AC_MSG_PACKET heartbeat;
	memset((BYTE*)&heartbeat, 0, sizeof(AC_MSG_PACKET));
	heartbeat.PacketID = CS_HEARTBEAT_MSG;
	heartbeat.USN = Instance->cPlayer->UserUSN + 21;
	// Send the packet to the server
	return Instance->SendPacketToServer((BYTE*)&heartbeat, sizeof(AC_MSG_PACKET));
}

bool She3aAC::SaveRoomPlayersInfo(ROOM_INFO Reason) {

	RoomInfoMessage.PacketID = CS_ROOMINFO_REPORT;
	RoomInfoMessage.USN = Instance->cPlayer->UserUSN + 21;
	RoomInfoMessage.PlayerType = Instance->cPlayer->PlayerType;
	RoomInfoMessage.Reason = Reason;
	RoomInfoMessage.CashAmount = Instance->cPlayer->ZpAmmount;
	RoomInfoMessage.GameMode = NONE;

	for (int i = 0; i < MAX_PLAYERS_IN_ROOM; i++) {
		strcpy_s(RoomInfoMessage.Players[i].Name, _xor("NOTUSED").c_str());
		RoomInfoMessage.Players[i].KillCount = 0;
	}

	if (!GameEngine || !GameEngine->CLTClientShell) return false;
	auto pLocal = GameEngine->CLTClientShell->GetLocalPlayer();
	if (!pLocal) return false;
	strcpy_s(RoomInfoMessage.Reporter, pLocal->szName);
	static DWORD dwRoomInfo = NULL;
	if (dwRoomInfo == NULL)
	{
		dwRoomInfo = ((DWORD)(GetModuleHandleA("CShell.dll")) + 0x1668228);
	}

	if (dwRoomInfo != NULL)
	{
		auto RoomManagerAddy = *reinterpret_cast<uintptr_t*>(dwRoomInfo);

		CRoomManager* room = reinterpret_cast<CRoomManager*>(RoomManagerAddy);
		if (room && room->RoomInfo)
		{
			RoomInfoMessage.GameMode = room->RoomInfo->GameMode;
		}
	}

	int iValidCount = 0;

	for (int i = 0; i < MAX_PLAYERS_IN_ROOM; ++i)
	{
		auto cPlayerX = GameEngine->CLTClientShell->GetPlayerByID(i);

		if (!cPlayerX || !cPlayerX->IsValidClient2()) continue;
		if (pLocal->bClientID == cPlayerX->bClientID) continue;


		if (_strnicmp(cPlayerX->szName, "BOT", 3) == 0) continue;
		if (_strnicmp(cPlayerX->szName, "[BOT]", 5) == 0) continue;


		strcpy_s(RoomInfoMessage.Players[iValidCount].Name, cPlayerX->szName);
		RoomInfoMessage.Players[iValidCount].KillCount = cPlayerX->Kills;
		RoomInfoMessage.Players[iValidCount].TeamID = cPlayerX->TeamID;

		iValidCount++;
	}

	//if (iValidCount < 4)
	//{
	//	printf("[-] Skipping Room Info: Not enough players (Count: %d)\n", iValidCount);
	//	return false;
	//}

	//printf("[+] Sending Room Players Info (Reason: %d | Count: %d)\n", Reason, iValidCount);

	AC_MSG_PACKET packet = { 0 };
	packet.PacketID = CS_ROOMINFO_REPORT_REQ;
	packet.USN = Instance->cPlayer->UserUSN + 21;

	return She3aAC::SendPacketToServer(reinterpret_cast<BYTE*>(&packet), sizeof(AC_MSG_PACKET));
}

bool She3aAC::SendRoomPlayersInfo() {
	SEND_ROOMINFO_REPORT EmptyStruct = { 0 };
	if (memcmp(&RoomInfoMessage, &EmptyStruct, sizeof(SEND_ROOMINFO_REPORT)) != 0)
		return She3aAC::SendPacketToServer(reinterpret_cast<BYTE*>(&RoomInfoMessage), sizeof(SEND_ROOMINFO_REPORT));
	else
		return false;
}

bool She3aAC::SendHeartbeatResponse() {
	AC_MSG_PACKET packet = { 0 };
	packet.PacketID = CS_HEARTBEAT_MSG;
	packet.USN = Instance->cPlayer->UserUSN + 21;
		return She3aAC::SendPacketToServer(reinterpret_cast<BYTE*>(&packet), sizeof(AC_MSG_PACKET));
}

bool She3aAC::SendScreenShot(SCREENSHOT_OPERATION Operation)
{
	std::vector<unsigned char> screenshotBuffer = Instance->cPlayer->cEngine->CaptureScreenshot();

	if (screenshotBuffer.empty()) {
		return false; 
	}

	unsigned int imageSize = static_cast<unsigned int>(screenshotBuffer.size());
	size_t structSize = sizeof(SEND_SCREENSHOT);
	size_t totalPacketSize = structSize + imageSize;

	std::vector<BYTE> fullPacket(totalPacketSize);
	SEND_SCREENSHOT* header = reinterpret_cast<SEND_SCREENSHOT*>(fullPacket.data());
	memset(header, 0, structSize);
	header->PacketID = CS_SCREENSHOT_DATA;
	header->USN = Instance->cPlayer->UserUSN + 21;
	header->Operation = Operation;
	header->ScreenshotSize = imageSize;

	memcpy(fullPacket.data() + structSize, screenshotBuffer.data(), imageSize);

	return SendPacketToServer(fullPacket.data(), totalPacketSize);
}

void She3aAC::StartLiveStream(int quality, int fps, bool audio) {
	LiveSteamData.lsQuality = quality;
	LiveSteamData.lsFPS = fps;
	LiveSteamData.lsAudio = audio;

	if (isLiveStreaming) return; 

	udpLiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpLiveSocket == INVALID_SOCKET) return;

	memset(&udpServerAddr, 0, sizeof(udpServerAddr));
	udpServerAddr.sin_family = AF_INET;
	udpServerAddr.sin_port = htons(1889);
	udpServerAddr.sin_addr.s_addr = inet_addr(GetCFServerIP().c_str());

	isLiveStreaming = true;
	LiveSteamData.hLiveStreamThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)LiveStreamWorker, this, 0, NULL);
}

void She3aAC::StopLiveStream() {
	isLiveStreaming = false;
	if (udpLiveSocket != INVALID_SOCKET) {
		closesocket(udpLiveSocket);
		udpLiveSocket = INVALID_SOCKET;
	}
	if (LiveSteamData.hLiveStreamThread) {
		WaitForSingleObject(LiveSteamData.hLiveStreamThread, 1000);
		CloseHandle(LiveSteamData.hLiveStreamThread);
		LiveSteamData.hLiveStreamThread = NULL;
	}
}

unsigned __stdcall She3aAC::LiveStreamWorker(void* lpParam) {
	She3aAC* ac = (She3aAC*)lpParam;
	unsigned int currentFrameID = 0;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID jpegClsid;
	ac->cPlayer->cEngine->GetEncoderClsid(_xor(L"image/jpeg").c_str(), &jpegClsid);

	Gdiplus::EncoderParameters encoderParameters;
	ULONG jpegQualityVal = 90;
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &jpegQualityVal;

	while (ac->isLiveStreaming) {
		auto frameStartTime = std::chrono::steady_clock::now();
		currentFrameID++;

		HWND hGame = ac->cPlayer->cEngine->GetProcessWindow();
		if (!hGame) hGame = GetDesktopWindow();
		HMONITOR hMonitor = MonitorFromWindow(hGame, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMonitor, &mi);

		int screenW = mi.rcMonitor.right - mi.rcMonitor.left;
		int screenH = mi.rcMonitor.bottom - mi.rcMonitor.top;

		int targetW = ac->LiveSteamData.lsQuality;
		if (targetW <= 0) targetW = 1280;
		int targetH = (screenH * targetW) / screenW;

		HDC hdcScreen = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, targetW, targetH);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);


		SetStretchBltMode(hdcMem, HALFTONE);
		SetBrushOrgEx(hdcMem, 0, 0, NULL);
		StretchBlt(hdcMem, 0, 0, targetW, targetH, hdcScreen, mi.rcMonitor.left, mi.rcMonitor.top, screenW, screenH, SRCCOPY);

		CURSORINFO cursorInfo = { sizeof(cursorInfo) };
		if (GetCursorInfo(&cursorInfo) && cursorInfo.flags == CURSOR_SHOWING) {
			ICONINFO iconInfo;
			if (GetIconInfo(cursorInfo.hCursor, &iconInfo)) {
				int xPos = cursorInfo.ptScreenPos.x - mi.rcMonitor.left - iconInfo.xHotspot;
				int yPos = cursorInfo.ptScreenPos.y - mi.rcMonitor.top - iconInfo.yHotspot;

				int scaledX = (xPos * targetW) / screenW;
				int scaledY = (yPos * targetH) / screenH;
				int scaledW = (GetSystemMetrics(SM_CXCURSOR) * targetW) / screenW;
				int scaledH = (GetSystemMetrics(SM_CYCURSOR) * targetH) / screenH;

				DrawIconEx(hdcMem, scaledX, scaledY, cursorInfo.hCursor, scaledW, scaledH, 0, NULL, DI_NORMAL);

				if (iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
				if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
			}
		}

		Gdiplus::Bitmap bitmap(hBitmap, NULL);
		IStream* pStream = NULL;
		std::vector<BYTE> jpegBuffer;

		if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK) {
			bitmap.Save(pStream, &jpegClsid, &encoderParameters);
			STATSTG stg;
			if (pStream->Stat(&stg, STATFLAG_NONAME) == S_OK) {
				jpegBuffer.resize((size_t)stg.cbSize.QuadPart);
				LARGE_INTEGER li = { 0 };
				pStream->Seek(li, STREAM_SEEK_SET, NULL);
				pStream->Read(jpegBuffer.data(), (ULONG)stg.cbSize.QuadPart, NULL);
			}
			pStream->Release();
		}

		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);

		if (!jpegBuffer.empty() && ac->udpLiveSocket != INVALID_SOCKET) {
			const int MAX_UDP_PAYLOAD = 60000;
			int totalSize = (int)jpegBuffer.size();
			int totalChunks = (totalSize + MAX_UDP_PAYLOAD - 1) / MAX_UDP_PAYLOAD;

			for (int i = 0; i < totalChunks; i++) {
				if (!ac->isLiveStreaming) break;

				int currentPayloadSize = (i == totalChunks - 1) ? (totalSize - (i * MAX_UDP_PAYLOAD)) : MAX_UDP_PAYLOAD;
				int packetSize = sizeof(UDP_FRAME_HEADER) + currentPayloadSize;

				std::vector<BYTE> sendBuffer(packetSize);
				UDP_FRAME_HEADER* hdr = (UDP_FRAME_HEADER*)sendBuffer.data();

				hdr->Type = UDP_TYPE_STREAM;
				hdr->USN = ac->cPlayer->UserUSN + 21;
				hdr->FrameID = currentFrameID;
				hdr->ChunkIdx = i;
				hdr->MaxChunks = totalChunks;
				hdr->PayloadLen = currentPayloadSize;

				memcpy(sendBuffer.data() + sizeof(UDP_FRAME_HEADER), jpegBuffer.data() + (i * MAX_UDP_PAYLOAD), currentPayloadSize);

				sendto(ac->udpLiveSocket, (const char*)sendBuffer.data(), packetSize, 0, (SOCKADDR*)&ac->udpServerAddr, sizeof(ac->udpServerAddr));
			}
		}

		auto frameEndTime = std::chrono::steady_clock::now();
		auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime - frameStartTime).count();

		int targetDelay = 1000 / (ac->LiveSteamData.lsFPS > 0 ? ac->LiveSteamData.lsFPS : 30);
		int sleepTime = targetDelay - (int)elapsedMs;

		if (sleepTime > 0) {
			Sleep(sleepTime);
		}
		else {
			Sleep(2);
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}
bool She3aAC::IsPlayerInGame()
{

	return *reinterpret_cast<int*>(cPlayer->cEngine->GetCShellDLL() + 0x016B3F58 + 0x7C);
}

// Make ErrMsg be Sent To server when wake up

void She3aAC::AfterLoginDetections()
{

	if (IsGameDebugged())
	{
		this->ReportError(CF_DEBUGGER_DETECTED);
	}

	Sleep(100);

	if (IsDllDetected())
	{
		this->ReportError(CHEAT_DLL_DETECTED);
	}

	Sleep(100);

	if (IsBasicPlayerInfoModified())
	{
		this->ReportError(BASICPLAYEREDIT_DETECTED);
	}

	Sleep(100);

	if (IsKernel32ModifiedDetected() || IsKernel32ModifiedDetectedWin7() || IsKernel32ModifiedDetectedWin7_2())
	{
		this->ReportError(KERNAL32_HOOK_DETECTED);
	}

	Sleep(100);

	if (IsNoBugDamageEnabled())
	{
		this->ReportError(NOBUGDAMAGE_DETECTED);
	}

	Sleep(100);

	if (IsXMouseDetected() || IsUMTScriptDetected() || IsProcessDetected())
	{
		this->ReportError(SCRIPT_PROGRAM_DETECTED);
	}

	Sleep(100);

	if (IsIllegalProcessDetected())
	{
		this->ReportError(ILLIGALPROGRAM_DETECTED);
	}

	Sleep(100);

	if (IsHookedCheatCMDDetected() || IsBitDefenderDetected())
	{
		this->ReportError(HOOKEDCMDCHEAT_DETECTED);
	}

	Sleep(100);

	if (IsSuperKill()) {

		this->ReportError(SUPERKILL_DETECTED);
	}

	Sleep(100);

	if (IsInvisibleCharacter())
	{
		this->ReportError(INVINSIBLECHAR_DETECTED);
	}

	Sleep(100);

	if (IsHGWorXignNotLoaded())
	{
		this->ReportError(HGWCBYPASS_DETECTED);
	}

	Sleep(100);

	if (AreMultipleClientRunning())
	{
		this->ReportError(MULTICLIENT_DETECTED);
	}

	Sleep(100);

	if (IsStringReloadEtcModified())
	{

		this->ReportError(WEAPON_STRING_MOD_DETECTED);

	}

	Sleep(100);

	if (ClientErrorBypassDetected())
	{
		this->ReportError(CLIENT_ERROR_BYPASS_DETECTED);
	}

	Sleep(100);

	if (IsSendToServerModified())
	{
		this->ReportError(S2S_MODIFY_DETECTED);
	}

	Sleep(100);

	if (IsD3DModified() || IsD3DModified2())
	{

		this->ReportError(D3D_MODIFY_DETECTED);

	}

	Sleep(100);

	if (IsD3D9Hooked() || IsD3D9Hooked2())
	{

		this->ReportError(D3D_HOOK_DETECTED);

	}

	// 10. External Overlay
	//if (IsExternalOverlayDetected())
	//{
	//	this->ReportError(EXTERNAL_OVERLAY_DETECTED);
	//}
	//Sleep(100);

}

void She3aAC::InGameDetections()
{
	Sleep(10000);

	SetAntiWallBan();

	Sleep(1000);

	if (IsWallArrayModified())
	{
		this->ReportError(WALLHACK_DETECTED);
	}

	Sleep(100);

	if (IsGameDebugged())
	{
		this->ReportError(CF_DEBUGGER_DETECTED);
	}

	Sleep(100);

	if (IsDllDetected())
	{
		this->ReportError(CHEAT_DLL_DETECTED);
	}

	Sleep(100);

	if (IsBasicPlayerInfoModified())
	{
		this->ReportError(BASICPLAYEREDIT_DETECTED);
	}

	Sleep(100);

	if (IsKernel32ModifiedDetected() || IsKernel32ModifiedDetectedWin7() || IsKernel32ModifiedDetectedWin7_2())
	{
		this->ReportError(KERNAL32_HOOK_DETECTED);
	}

	Sleep(100);

	if (IsNoBugDamageEnabled())
	{
		this->ReportError(NOBUGDAMAGE_DETECTED);
	}

	Sleep(100);

	if (IsXMouseDetected() || IsUMTScriptDetected() || IsProcessDetected())
	{
		this->ReportError(ANTISHAKESCHOOK_DETECTED);
	}

	Sleep(100);

	if (IsIllegalProcessDetected())
	{
		this->ReportError(ILLIGALPROGRAM_DETECTED);
	}

	Sleep(100);

	if (IsHookedCheatCMDDetected() || IsBitDefenderDetected())
	{
		this->ReportError(HOOKEDCMDCHEAT_DETECTED);
	}

	Sleep(100);

	if (IsSuperKill())
	{
		this->ReportError(SUPERKILL_DETECTED);
	}

	Sleep(100);

	if (IsInvisibleCharacter())
	{
		this->ReportError(INVINSIBLECHAR_DETECTED);
	}

	Sleep(100);

	if (IsHGWorXignNotLoaded())
	{
		this->ReportError(HGWCBYPASS_DETECTED);
	}

	Sleep(100);

	if (AreMultipleClientRunning())
	{
		this->ReportError(MULTICLIENT_DETECTED);
	}

	Sleep(100);

	if (IsStringReloadEtcModified())
	{
		this->ReportError(WEAPON_STRING_MOD_DETECTED);
	}

	Sleep(100);

	if (ClientErrorBypassDetected())
	{
		this->ReportError(CLIENT_ERROR_BYPASS_DETECTED);
	}

	Sleep(100);

	if (IsSendToServerModified())
	{
		this->ReportError(S2S_MODIFY_DETECTED);
	}

	Sleep(100);

	if (IsD3DModified() || IsD3DModified2())
	{
		this->ReportError(D3D_MODIFY_DETECTED);
	}

	Sleep(100);

	if (IsD3D9Hooked() || IsD3D9Hooked2())
	{
		this->ReportError(D3D_HOOK_DETECTED);
	}

	Sleep(100);

	if (IsExternalOverlayDetected())
	{
	//	this->ReportError(EXTERNAL_OVERLAY_DETECTED);
	}
	Sleep(100);

	if (IsCHBypassed())
	{
	//	this->ReportError(CODEHUNTERBYPASS_DETECTED);
	}

	Sleep(100);

	if (IsBanPacketBypassed())
	{
	//	this->ReportError(BANPACKETBYPASS_DETECTED);
	}

	Sleep(100);

	if (IsMTPPerfectRecoil())
	{
	//	this->ReportError(MTPPERFECTRECOIL_DETECTED);
	}

	Sleep(100);

	if (IsNadeBypassed())
	{
	//	this->ReportError(GERNADEBYPASS_DETECTED);
	}

	Sleep(100);

	if (IsNzDBypassed())
	{
	//	this->ReportError(NOZONEDAMAGE_DETECTED);
	}

	Sleep(100);

	if (IsRemoteHooked())
	{
	//	this->ReportError(REMOTE_HOOK_DETECTED);
	}

	Sleep(100);

	if (IsStw())
	{
	//	this->ReportError(STW_DETECTED);
	}

	Sleep(100);

	if (IsGlowHack())
	{
	//	this->ReportError(GLOWHACK_DETECTED);
	}

	Sleep(100);

	if (WeaponButesCheckPattern())
	{
		//this->ReportError(WEAPONBUTE_MODIFY_DETECTED);
	}

	Sleep(100);
}

unsigned __stdcall She3aAC::HandleAntiCheatDetections()
{

	while (true) {

		if (Instance->WasFlaggedAsCheater) {
			Sleep(1000);
			continue;
		}

		if (Instance->IsPlayerInGame())
			Instance->InGameDetections();
		else {
			Instance->AfterLoginDetections();
		}
		Sleep(200);
	}

	return 0;
}

bool She3aAC::ConnectToServer()
{

	VIRTUALIZER_START

		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		VIRTUALIZER_END
			return false;
	}

	//sin.sin_addr.s_addr = inet_addr(Instance->GetCFServerIP().c_str());
	sin.sin_addr.s_addr = inet_addr(Instance->GetCFServerIP().c_str());
	sin.sin_family = AF_INET;
	sin.sin_port = htons(1888);
	connect(sock, (SOCKADDR*)&sin, sizeof(sin));
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		VIRTUALIZER_END
			return false;
	}

	VIRTUALIZER_END

		return true;
}

const size_t MAX_CHUNK_SIZE = 8192;

bool She3aAC::SendPacketToServer(BYTE* datas, size_t len)
{
	ULONGLONG waitStart = GetTickCount();
	const DWORD MAX_WAIT_MS = 15000;

	while (Instance->IsSendingData) {
		if (GetTickCount() - waitStart > MAX_WAIT_MS) {
			Instance->IsSendingData = false;
			break;
		}
		Sleep(2);
	}

	if (sock == INVALID_SOCKET) return false;
	Instance->IsSendingData = true;

	VIRTUALIZER_START

		auto now = std::chrono::system_clock::now();
	uint64_t localNow = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

	long long offset = (long long)Instance->ServerTimestamp - (long long)Instance->LocalTimestamp;
	long long finalCalculatedTime = (long long)localNow + offset;
	unsigned long long syncedTimestamp = (finalCalculatedTime < 0) ? 0 : (unsigned long long)finalCalculatedTime;

	size_t headerSize = sizeof(PACKET_HEADER);
	size_t payloadTotalSize = len - headerSize;
	PACKET_HEADER* originalHeader = (PACKET_HEADER*)datas;

	if (len <= (MAX_CHUNK_SIZE + headerSize)) {
		originalHeader->CurCount = 0;
		originalHeader->ExtCount = 0;
		originalHeader->Timestamp = syncedTimestamp; 
		originalHeader->Len = (unsigned int)payloadTotalSize;

		originalHeader->CRC = CRC32::Compute(datas + headerSize, payloadTotalSize);

		EncryptDatas(datas, len);
		send(sock, (const char*)datas, (int)len, 0);
	}
	else {
		unsigned short totalChunks = (unsigned short)((payloadTotalSize + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE);
		BYTE* payloadPtr = datas + headerSize;
		size_t remainingSize = payloadTotalSize;

		for (unsigned short i = 1; i <= totalChunks; i++) {
			size_t currentChunkSize = (remainingSize > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : remainingSize;
			size_t totalPacketSize = headerSize + currentChunkSize;

			std::vector<BYTE> chunk(totalPacketSize);
			PACKET_HEADER* ch = (PACKET_HEADER*)chunk.data();

			ch->PacketID = originalHeader->PacketID;
			ch->CurCount = i;
			ch->ExtCount = totalChunks;
			ch->Timestamp = syncedTimestamp; 
			ch->Len = (unsigned int)currentChunkSize;

			memcpy(chunk.data() + headerSize, payloadPtr, currentChunkSize);

			ch->CRC = CRC32::Compute(chunk.data() + headerSize, currentChunkSize);

			EncryptDatas(chunk.data(), totalPacketSize);

			if (send(sock, (const char*)chunk.data(), (int)totalPacketSize, 0) == SOCKET_ERROR) {
				Instance->IsSendingData = false;
				VIRTUALIZER_END
					return false;
			}

			payloadPtr += currentChunkSize;
			remainingSize -= currentChunkSize;
		}
	}

	Instance->IsSendingData = false;
	VIRTUALIZER_END
		return true;
}

unsigned __stdcall She3aAC::NetworkListener(void* lpParam)
{
	BYTE rawBuffer[16384];
	int receivedBytes = 0;
	She3aAC* ac = She3aAC::Instance;

	std::vector<BYTE> streamBuffer;
	std::map<PACKET_ID, std::vector<BYTE>> reassemblyMap;


	bool isHandshakeDone = false;
	while (!isHandshakeDone)
	{
		receivedBytes = recv(ac->sock, (char*)rawBuffer, sizeof(rawBuffer), 0);
		if (receivedBytes <= 0) return 0;

		streamBuffer.insert(streamBuffer.end(), rawBuffer, rawBuffer + receivedBytes);

		if (streamBuffer.size() >= sizeof(PACKET_HEADER))
		{
			BYTE tempHeader[sizeof(PACKET_HEADER)];
			memcpy(tempHeader, streamBuffer.data(), sizeof(PACKET_HEADER));
			ac->DecryptDatas(tempHeader, sizeof(PACKET_HEADER));

			PACKET_HEADER* header = (PACKET_HEADER*)tempHeader;
			size_t fullSize = sizeof(PACKET_HEADER) + header->Len;

			if (streamBuffer.size() >= fullSize)
			{
				std::vector<BYTE> authPacket(fullSize);
				memcpy(authPacket.data(), streamBuffer.data(), fullSize);
				streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + fullSize);

				ac->DecryptDatas(authPacket.data(), fullSize);
				PACKET_HEADER* finalHeader = (PACKET_HEADER*)authPacket.data();

				if (finalHeader->PacketID == SC_AUTHDATA_REQ)
				{
					auto now = std::chrono::system_clock::now();
					uint64_t localNow = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

					uint64_t diff = (localNow > finalHeader->Timestamp) ? (localNow - finalHeader->Timestamp) : (finalHeader->Timestamp - localNow);

					if (diff > (12 * 3600))
					{
						ac->cPlayer->cEngine->ShowMessage(_xor("Security Error: System time mismatch. Please sync your Windows clock.").c_str());
						closesocket(ac->sock);
						ac->sock = INVALID_SOCKET;
						return 0;
					}

					ac->ServerTimestamp = finalHeader->Timestamp;
					ac->LocalTimestamp = localNow;

				//	cout << "[+] Handshake Success: Server Time Synced." << endl;
				//	cout << "[+] RECV : SC_AUTHDATA_REQ (Initial)" << endl;

					if (!ac->SendAuthRequest()) {
						ac->cPlayer->cEngine->ShowMessage(_xor("Connection to server lost (0x1).").c_str());
						return 0;
					}

					isHandshakeDone = true;
				}
				else
				{
					ac->cPlayer->cEngine->ShowMessage(_xor("Security Violation: Invalid Handshake Sequence.").c_str());
					closesocket(ac->sock);
					ac->sock = INVALID_SOCKET;
					return 0;
				}
			}
		}
	}


	while (true)
	{
		receivedBytes = recv(ac->sock, (char*)rawBuffer, sizeof(rawBuffer), 0);

		if (receivedBytes <= 0)
		{
			closesocket(ac->sock);
			ac->sock = INVALID_SOCKET;
			if (!ac->WasFlaggedAsCheater)
				ac->cPlayer->cEngine->ShowMessage(_xor("Connection to server lost.").c_str());
			break;
		}

		streamBuffer.insert(streamBuffer.end(), rawBuffer, rawBuffer + receivedBytes);

		while (streamBuffer.size() >= sizeof(PACKET_HEADER))
		{
			BYTE tempHeader[sizeof(PACKET_HEADER)];
			memcpy(tempHeader, streamBuffer.data(), sizeof(PACKET_HEADER));
			ac->DecryptDatas(tempHeader, sizeof(PACKET_HEADER));

			PACKET_HEADER* header = (PACKET_HEADER*)tempHeader;
			size_t fullPacketSize = sizeof(PACKET_HEADER) + header->Len;

			if (streamBuffer.size() >= fullPacketSize)
			{
				std::vector<BYTE> fullPacket(fullPacketSize);
				memcpy(fullPacket.data(), streamBuffer.data(), fullPacketSize);
				streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + fullPacketSize);

				ac->DecryptDatas(fullPacket.data(), fullPacketSize);

				PACKET_HEADER* finalHeader = (PACKET_HEADER*)fullPacket.data();
				BYTE* payload = fullPacket.data() + sizeof(PACKET_HEADER);

				unsigned long computedCRC = CRC32::Compute(payload, finalHeader->Len);
				if (finalHeader->CRC != computedCRC) continue;

				auto now = std::chrono::system_clock::now();
				uint64_t localNow = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

				long long offset = (long long)ac->ServerTimestamp - (long long)ac->LocalTimestamp;
				uint64_t expectedServerTime = (uint64_t)((long long)localNow + offset);

				uint64_t drift = (expectedServerTime > finalHeader->Timestamp) ?
					(expectedServerTime - finalHeader->Timestamp) :
					(finalHeader->Timestamp - expectedServerTime);

				if (drift > 120) continue; 

				if (finalHeader->PacketID == SC_AUTHDATA_REQ)
				{
					continue;
				}

				std::vector<BYTE> processedBuffer;
				if (finalHeader->ExtCount == 0) {
					processedBuffer = fullPacket;
				}
				else {
					auto& storage = reassemblyMap[finalHeader->PacketID];
					if (finalHeader->CurCount == 1) storage.clear();
					storage.insert(storage.end(), payload, payload + finalHeader->Len);

					if (finalHeader->CurCount == finalHeader->ExtCount) {
						processedBuffer.resize(sizeof(PACKET_HEADER) + storage.size());
						memcpy(processedBuffer.data(), fullPacket.data(), sizeof(PACKET_HEADER));
						PACKET_HEADER* ph = (PACKET_HEADER*)processedBuffer.data();
						ph->Len = (unsigned int)storage.size();
						ph->CurCount = 0; ph->ExtCount = 0;
						memcpy(processedBuffer.data() + sizeof(PACKET_HEADER), storage.data(), storage.size());
						storage.clear();
					}
					else continue;
				}

				PACKET_HEADER* dispatchHeader = (PACKET_HEADER*)processedBuffer.data();
				BYTE* dispatchBuf = processedBuffer.data();

				switch (dispatchHeader->PacketID)
				{
				case SC_SCREENSHOT_REQ:
				{
				//	cout << "[+] RECV : SC_SCREENSHOT_REQ" << endl;
					SC_SCREENSHOT_REQUEST* scReq = (SC_SCREENSHOT_REQUEST*)dispatchBuf;
					SCREENSHOT_OPERATION reqOp = scReq->Operation;
					if (reqOp == REPORT_REQ)
						ac->cPlayer->isBannedScReqRecved = true;

					std::thread([reqOp]() {
						if (!She3aAC::Instance->SendScreenShot(reqOp)) {
						//	cout << "[-] Failed to capture or send screenshot." << endl;
						}
						}).detach();
					break;
				}
				case SC_HEARTBEAT_REQ:
				{
				//	cout << "[+] RECV : SC_HEARTBEAT_REQ" << endl;
					if (!ac->SendHeartbeatResponse())
						ac->cPlayer->cEngine->ShowMessage(_xor("Connection to server lost (0x3).").c_str());
					break;
				}

				case SC_APPROVE_ROOMINFO:
				{
				//	cout << "[+] RECV : SC_APPROVE_ROOMINFO" << endl;
					if (ac->SendRoomPlayersInfo())
						ac->cPlayer->cEngine->ShowNormalMessage(ac->RmInfoMsg_S);
					else
						ac->cPlayer->cEngine->ShowNormalMessage(ac->RmInfoMsg_F);
					break;
				}
				case SC_APPROVE_ERROR:
				{
				//	cout << "[+] RECV : SC_APPROVE_ERROR" << endl;
					if (ac->SendErrorCode())
						ac->IsSentErrorCode = true;
					else
						ac->IsRecvedErrorReportResult = true;
					break;
				}
				case SC_RESPONSE_AUTHDATA:
				{
				//	cout << "[+] RECV : SC_RESPONSE_AUTHDATA" << endl;
					AUTH_REQUEST_RESPONSE* authResp = (AUTH_REQUEST_RESPONSE*)dispatchBuf;
					if (ac->cPlayer->UserUSN == authResp->USN) {

						if (authResp->Version == acVersion)
						{
							ac->cPlayer->PlayerType = authResp->PlayerType;
							ac->cPlayer->IsAuthorized = true;
						}
						else {
							ac->cPlayer->cEngine->ShowMessage(_xor("Client Version Mismatch.\n\nPlease Update The Game To The Last Version.").c_str());
							break;

						}

					}
					break;
				}
				case SC_RESPONSE_ERROR_REPORT:
				{
			//		cout << "[+] RECV : SC_RESPONSE_ERROR_REPORT" << endl;
					AC_MSG_PACKET* errorMsg = (AC_MSG_PACKET*)dispatchBuf;
					if (ac->cPlayer->UserUSN == errorMsg->USN)
					{
						ac->IsRecvedErrorReportResult = true;
					}
					break;
				}

				case SC_LIVESHARE_CMD:
				{
				//	cout << "[+] RECV : SC_LIVESHARE_CMD" << endl;

					SC_LIVESHARE_COMMAND* cmd = (SC_LIVESHARE_COMMAND*)dispatchBuf;
					if (cmd->Action == LS_START || cmd->Action == LS_UPDATE) {
						ac->StartLiveStream(cmd->Quality, cmd->TargetFPS, cmd->EnableAudio);
					}
					else if (cmd->Action == LS_STOP) {
						ac->StopLiveStream();
					}
					break;
				}

				default:
				{
				//	printf("[+] RECV : SC_UNKOWN [PACKET_ID : %s]\n" , (std::to_string(dispatchHeader->PacketID)).c_str() );
				
					break;
				}
				}
			}
			else break;
		}
		memset(rawBuffer, 0, sizeof(rawBuffer));
		Sleep(5);
	}
	return 0;
}

void She3aAC::Run()
{

	//Instance->cPlayer->cEngine->ShowNormalMessage(_xor("Welcome to New She3aAC System\n\nPlease Login To Proceed.").c_str());

//	printf("Waiting for game to initialize player data...\n");

	cPlayer->GetUserData();


	if(!Instance->ConnectToServer())
		Instance->cPlayer->cEngine->ShowMessage(_xor("Failed To Connect To Server (0xDEAD).").c_str());

	//printf("Connected to server, starting network listener thread...\n");


	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GarnetAndProxyWorker, NULL, 0, NULL);

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)NetworkListener, NULL, 0, NULL);
	DWORD startTime = GetTickCount();
	const DWORD AUTH_TIMEOUT_MS = 15000;

	while (!Instance->cPlayer->IsAuthorized)
	{
		if (GetTickCount() - startTime > AUTH_TIMEOUT_MS)
		{
			Instance->cPlayer->cEngine->ShowMessage(_xor("Authorization failed. Server did not respond.").c_str());
			return;
		}


		if (Instance->sock == INVALID_SOCKET) {
			Instance->cPlayer->cEngine->ShowMessage(_xor("Connection lost during authorization.").c_str());
			return;
		}

		Sleep(100);
	}

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)HandleAntiCheatDetections, NULL, 0, NULL);

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GameFlowManager, NULL, 0, NULL);

	while (true)
	{

		if (Instance->sock == INVALID_SOCKET || !Instance->cPlayer->IsAuthorized)
		{
			Instance->cPlayer->cEngine->ShowMessage(_xor("VORTEX: Connection to security server lost.\n\n Closing game for your protection.").c_str());

			//TerminateProcess(GetCurrentProcess(), 0);
			break;
		}

		Sleep(1000);



		//if (Instance->IsExternalOverlayDetected())
		//{
		//	this->ReportError(EXTERNAL_OVERLAY_DETECTED);

		//}

	}
}

bool She3aAC::Init()
{


//	printf("Initializing She3aAC...\n");

	bool hookSuccess = VortexEncryption((DWORD_PTR)GetModuleHandleA(_xor("crossfire.exe").c_str()));


	ACCreateDirectory = (_CreateDirectoryA)ACGetProcAddress(GetModuleHandleA(kernel32), CreateDirectoryA_func);
	if (!ACCreateDirectory)
		return false;
	ACExitProcess = (_ExitProcess)ACGetProcAddress(GetModuleHandleA(kernel32), ExitProcess_func);
	if (!ACExitProcess)
		return false;
	ACMessageBox = (_MessageBox)ACGetProcAddress(GetModuleHandleA(user32), MessageBox_func);
	if (!ACMessageBox)
		return false;
	NtQuerySystemInformation = (_NtQuerySystemInformation)ACGetProcAddress(GetModuleHandleA(ntdll), NtQuerySystemInformation_func);
	if (!NtQuerySystemInformation)
		return false;
	ACVirtualProtect = (_VirtualProtect)ACGetProcAddress(GetModuleHandleA(kernel32), VirtualProtect_func);
	if (!ACVirtualProtect)
		return false;
#ifdef AC_LOG_ENABLED
	ClearLog();
#endif





	if (this->IsTestSigningEnabled())
	{

		this->ReportError(TESTSIGN_ENABLED);
		return false;
	}

	if (this->IsGameDebugged())
	{



		this->ReportError(DEBUG_DETECT_BANKEY);
		return false;
	}

#ifdef AC_LOG_ENABLED
	Log("Init Done!\n");
#endif

	if (!hookSuccess) {
		return false;
	}

	InitializeHooks();

	Sleep(1000);

	if (!ExecuteAutoLogin())
	{

		printf("1234\n");

	}

	while (!(DWORD)GetModuleHandleA(GAME_CSHELL)) {
		Sleep(1000);
	}
	Instance->cPlayer->cEngine->Init();
		Instance->cPlayer->cEngine->InstallHooks(cPlayer->cEngine);
		cPlayer->cEngine->pInstance = Instance->cPlayer;
		InitOwnDllInfo();
		ItemLimitPatch();




	//Injection();
	//cout << "Init Done!" << endl;
	return true;
}