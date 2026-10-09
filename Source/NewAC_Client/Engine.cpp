#include "SAC.h"
#include <random>
#include "MemoryAddr.h"
#include "md6.h"
#include <memory>
#include <intrin.h>
#include <wtypes.h>
#include <Shlwapi.h>

CEngine* CEngine::Instance = nullptr;


using namespace Microsoft::WRL;
using namespace std;
PlayerInfo* CEngine::pInstance = nullptr;

ComPtr<ICoreWebView2Controller> CEngine::webviewController;
ComPtr<ICoreWebView2> CEngine::webviewWindow;

BOOL CALLBACK CEngine::EnumProc(HWND hWnd, LPARAM lParam) {
	EnumData& data = *(EnumData*)lParam;
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	if (data.dwProcessId == dwProcessId && GetConsoleWindow() != hWnd) {
		if (GetWindow(hWnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hWnd)) {
			data.hWnd = hWnd;
			return FALSE; 
		}
	}
	return TRUE;
}

HWND CEngine::GetProcessWindow() {
	EnumData data = { GetCurrentProcessId(), 0 };
	EnumWindows(EnumProc, (LPARAM)&data);
	return data.hWnd;
}

void CEngine::DisableTopMost() {

	HWND gameHwnd = NULL;
	if (gameHwnd == NULL) {
		gameHwnd = GetProcessWindow();
	}

	if (gameHwnd != NULL) {
		SetWindowPos(gameHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

std::string CEngine::GetModuleNameFromAddress(DWORD address) {
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

std::wstring CEngine::ReadRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName) {

	HKEY key;
	if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
		WCHAR buffer[1024];
		DWORD bufferSize = sizeof(buffer);
		if (RegQueryValueExW(key, valueName.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
			RegCloseKey(key);
			return std::wstring(buffer);
		}
		RegCloseKey(key);
	}

	return L"";

}

bool CEngine::WriteRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value) {

	HKEY key;
	if (RegCreateKeyExW(hKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {
		if (RegSetValueExW(key, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), (value.size() + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
			RegCloseKey(key);
			return true;
		}
		RegCloseKey(key);
	}

	return false;
}

int CEngine::GenerateRandomNumber() {
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> distribution(0, 9999999);
	int randomNumber = distribution(generator);
	return randomNumber;
}

DWORD CEngine::GetVolumeID()
{
	DWORD VolumeSerialNumber;

	BOOL GetVolumeInformationFlag = GetVolumeInformationA(_xor("C:\\").c_str(), 0, 0, &VolumeSerialNumber, 0, 0, 0, 0);

	if (GetVolumeInformationFlag)
		return VolumeSerialNumber;
}

char16_t CEngine::GetCpuID()
{
	int cpuinfo[4] = { 0, 0, 0, 0 };
	__cpuid(cpuinfo, 0);
	char16_t hash = 0;
	char16_t* ptr = (char16_t*)(&cpuinfo[0]);
	for (char32_t i = 0; i < 8; i++)
		hash += ptr[i];

	return hash;
}

std::string CEngine::GetUUID()
{
	HANDLE h = CreateFileA(_xor("\\\\.\\PhysicalDrive0").c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (h == INVALID_HANDLE_VALUE)
		return {};

	std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) { CloseHandle(handle); } };

	STORAGE_PROPERTY_QUERY storagePropertyQuery{};
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader{};
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
		return {};

	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	std::unique_ptr<BYTE[]> pOutBuffer{ new BYTE[dwOutBufferSize]{} };

	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer.get(), dwOutBufferSize, &dwBytesReturned, NULL))
		return {};

	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer.get());
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset == 0)
		return {};

	std::string serialNumber = reinterpret_cast<const char*>(pOutBuffer.get() + dwSerialNumberOffset);
	std::string HWID = serialNumber;
	HWID += GetCpuID();
	HWID += GetVolumeID();
	return GetMD5String(HWID);
}

std::string CEngine::GetGUID() {

	if (!CShell)

		return "";

	int Info_1;
	int Info_2;
	int Info_3;
	int Info_4;
	int Info_5;
	int Info_6;
	int AUTH_DATA;
	std::string CSHELL_MAC;
	int HWID;
	SYSTEM_INFO siSysInfo;

	GetSystemInfo(&siSysInfo);

	Info_1 = siSysInfo.dwOemId;
	Info_2 = siSysInfo.dwNumberOfProcessors;
	Info_3 = siSysInfo.dwProcessorType;
	Info_4 = siSysInfo.dwActiveProcessorMask;
	Info_5 = siSysInfo.wProcessorLevel;
	Info_6 = siSysInfo.wProcessorRevision;

	int HWID_Calculator[6] = { Info_1, Info_2, Info_3, Info_4, Info_5, Info_6 };
	HWID = HWID_Calculator[0] * HWID_Calculator[1] * HWID_Calculator[2] * HWID_Calculator[3] * HWID_Calculator[4] * HWID_Calculator[5] * 2 * 4 * 8 * 16 * 32 * 64 * 120;
	if (HWID == 0) { HWID = GenerateRandomNumber(); }

	while (1) {
		CSHELL_MAC = RPMS(CShell + OFF_CSHELL_CLIENT_MAC);
		AUTH_DATA = RPM<int>(CShell + OFF_CSHELL_CLIENT_AUTH_DATA);
		if (CSHELL_MAC != "") {
			break;
		}
	}

	if (AUTH_DATA == 0) { AUTH_DATA = GenerateRandomNumber(); }
	else { AUTH_DATA = RPM<int>(CShell + OFF_CSHELL_CLIENT_AUTH_DATA); }

	std::string HWID2 = std::to_string(HWID) + "-" + CSHELL_MAC + "-" + std::to_string(AUTH_DATA);

	// Breaking down the problematic line
	std::wstring subKey = _xor(L"Software\\Microsoft").c_str();
	std::wstring valueName = _xor(L"GameBar").c_str();
	/* this is line 440 */ std::wstring regValue2 = ReadRegistryValue(HKEY_CURRENT_USER, subKey, valueName);

	if (regValue2.empty()) {
		bool success = WriteRegistryValue(HKEY_CURRENT_USER, subKey, valueName, std::wstring(HWID2.begin(), HWID2.end()));
		if (!success) {

			return "";
		}
	}
	else {
		std::string regValueStr(regValue2.begin(), regValue2.end());
		if (HWID2 != regValueStr) {

			return regValueStr;
		}
		else {

			return HWID2;
		}
	}


}

std::string CEngine::Clean(const std::string& input)
{
	std::string result;
	for (char c : input) {
		if (isprint(static_cast<unsigned char>(c))) {
			result += c;
		}
	}
	return result;
}

int CEngine::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	UINT num = 0;
	UINT size = 0;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	Gdiplus::ImageCodecInfo* imageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (imageCodecInfo == NULL)
		return -1;

	Gdiplus::GetImageEncoders(num, size, imageCodecInfo);

	for (UINT j = 0; j < num; ++j) {
		if (wcscmp(imageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = imageCodecInfo[j].Clsid;
			free(imageCodecInfo);
			return j;
		}
	}

	free(imageCodecInfo);
	return -1;
}



BOOL CALLBACK CEngine::GlobalEnumWindowsProc(HWND hwnd, LPARAM lParam) {
	auto* windowList = reinterpret_cast<std::vector<WindowAffinityData>*>(lParam);

	if (IsWindowVisible(hwnd)) {
		DWORD affinity = 0;
		if (GetWindowDisplayAffinity(hwnd, &affinity) && affinity != WDA_NONE) {
			windowList->push_back({ hwnd, affinity });
			SetWindowDisplayAffinity(hwnd, WDA_NONE);
		}
	}
	return TRUE;
}

std::vector<unsigned char> CEngine::CaptureScreenshot() {
	std::vector<unsigned char> buffer;
	std::vector<WindowAffinityData> changedWindows;

	if (g_CaptureEverything) {
		EnumWindows(GlobalEnumWindowsProc, reinterpret_cast<LPARAM>(&changedWindows));
		Sleep(15);
	}

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	{
		HDC hdcScreen = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

		BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY | CAPTUREBLT);

		Gdiplus::Bitmap bitmap(hBitmap, NULL);
		CLSID clsid;
		GetEncoderClsid(_xor(L"image/jpeg").c_str(), &clsid);

		IStream* pStream = NULL;
		if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK) {
			bitmap.Save(pStream, &clsid, NULL);
			STATSTG stg;
			if (pStream->Stat(&stg, STATFLAG_NONAME) == S_OK) {
				buffer.resize((size_t)stg.cbSize.QuadPart);
				LARGE_INTEGER li = { 0 };
				pStream->Seek(li, STREAM_SEEK_SET, NULL);
				pStream->Read(buffer.data(), (ULONG)stg.cbSize.QuadPart, NULL);
			}
			pStream->Release();
		}

		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	for (const auto& window : changedWindows) {
		if (IsWindow(window.hwnd)) {
			SetWindowDisplayAffinity(window.hwnd, window.originalAffinity);
		}
	}

	return buffer;
}

bool CEngine::Init()
{
	DWORD startTime = GetTickCount();

	while (GetTickCount() - startTime < 15000)
	{
		CShell = (DWORD)GetModuleHandleA(_xor("CShell.dll").c_str());
		ObjectDll = (DWORD)GetModuleHandleA(_xor("object.dll").c_str());
		ClientFX = (DWORD)GetModuleHandleA(_xor("ClientFx.fxd").c_str());

		pGameDervice = (DWORD)((CrossFire + (uGameDervice + 1)));

		g_pd3dDevice = GetDevice();

		if (CShell && g_pd3dDevice)
		{
			return true; 
		}

		Sleep(100);
	}

	return false;
}

CEngine::CEngine(PlayerInfo* pInfo)
{

	pInstance = pInfo;
	printf("[*] Initializing Engine...\n");
	CrossFire = (DWORD)GetModuleHandleA(_xor("crossfire.exe").c_str());
	webviewController = nullptr;
	webviewWindow = nullptr;

	printf("[+] Engine Initialized Successfully!\n");

}

void CEngine::RunCrashPreventerLogic() {
	uintptr_t mainAddr = ReadMemSafe<uintptr_t>(CShell + ADDR_GAMESRV_PORT_POINTER);
	if (mainAddr != 0) {
		GameServer* server = reinterpret_cast<GameServer*>(mainAddr);
		if (!IsBadReadPtr(server, sizeof(GameServer))) {
			if (server->port == 10009) {
				server->port = 26909;
			}
		}
	}
}

bool loadedobjectdll = false;
bool topmostfixed = false;

 unsigned __stdcall She3aAC::GameFlowManager()
{

	while (true) {

		if (Instance->cPlayer->cEngine->GetObjectDLL())
		{
			if (!loadedobjectdll)
			{
				if (Instance->ItemLimitPatch_inGame())
					loadedobjectdll = true;
			}
			else
			{

					if (!topmostfixed && Instance->IsPlayerInGame())
					{

						Instance->cPlayer->cEngine->DisableTopMost();
						topmostfixed = true;
					}
					else if (topmostfixed && Instance->IsPlayerInGame())
					{
						if ((Instance->cPlayer->PlayerType == GM_PLAYER || Instance->cPlayer->PlayerType == ADMIN_PLAYER) && Instance->cPlayer->WantToSendRoomInfo)
						{
							Instance->SaveRoomPlayersInfo(GM_EVENT);
								Instance->cPlayer->cEngine->ShowNormalMessage(Instance->RmInfoMsg_S);


					

							Instance->cPlayer->WantToSendRoomInfo = false;
						}

					}


			}
		}
		else {

			loadedobjectdll = false;
			topmostfixed = false;
		}

		Sleep((loadedobjectdll && topmostfixed) ? 200 : 15);
	}

}

 unsigned __stdcall WINAPI She3aAC::GarnetAndProxyWorker(LPVOID lpParam) {


	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	uintptr_t gARAddr = 0;
	bool GarPopupShowed = false;
	MSG msg;

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (Instance->cPlayer->cEngine->GetCShellDLL() != 0) {
			uintptr_t RoomInfo = Instance->cPlayer->cEngine->ReadMemSafe<uintptr_t>(Instance->cPlayer->cEngine->GetCShellDLL() + ADDR_LOBBY_POINTER);

			if (RoomInfo == GAME_LOBBY::SERVER_SELECT) {
				//Instance->cPlayer->cEngine->RunCrashPreventerLogic();
			}
			/*
			else if (RoomInfo == GAME_LOBBY::BLACK_MARCKET) {
				/
				int UserUSN = Instance->cPlayer->UserUSN;
				std::string EncodedPWD = Instance->cPlayer->cEngine->UrlEncode(Instance->cPlayer->UserPWD);

				std::wstring BaseLink = _xor(L"http://185.248.33.207/g.php?u=").str();
				std::wstring GarLink = BaseLink + std::to_wstring(UserUSN) + L"&p=" + std::wstring(EncodedPWD.begin(), EncodedPWD.end());
				Instance->cPlayer->cEngine->RunGarnetLogic(GarLink, GarPopupShowed, gARAddr, Instance->cPlayer->cEngine->g_pd3dDevice);
				
			}

			else {
				if (GarPopupShowed || Instance->cPlayer->cEngine->g_PopupInterrupted) {
					if (Instance->cPlayer->cEngine->webviewController) Instance->cPlayer->cEngine->webviewController->put_IsVisible(FALSE);
					Instance->cPlayer->cEngine->g_CurrentVisibleState = false;
					GarPopupShowed = false;
					Instance->cPlayer->cEngine->g_PopupInterrupted = false;
				}
				
			}

			*/
		}

		Sleep(100);
	}

	CoUninitialize();
	return 0;
}

__declspec(noinline) IDirect3DDevice9* CEngine::GetDevice() {
	DWORD* dwPointer = (DWORD*)pGameDervice;

	if (!dwPointer)
		return NULL;

	if (!*dwPointer)
		return NULL;

	DWORD* dwPointerTwo = (DWORD*)*dwPointer;

	if (!dwPointerTwo)
		return NULL;

	if (!*dwPointerTwo)
		return NULL;

	DWORD* dwPointerThree = (DWORD*)*dwPointerTwo;

	if (!dwPointerThree)
		return NULL;

	if (!*dwPointerThree)
		return NULL;

	return (IDirect3DDevice9*)*dwPointerThree;
}

std::wstring CEngine::GetGameDir() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	PathRemoveFileSpecW(buffer);
	return std::wstring(buffer);
}

void CEngine::KillOrphanedWebViewProcesses() {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) return;

	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(pe);

	if (Process32FirstW(hSnap, &pe)) {
		do {
			if (wcsstr(pe.szExeFile, _xor(L"msedge.exe").str().c_str()) || wcsstr(pe.szExeFile, _xor(L"WebView2").str().c_str())) {
				HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
				if (hProc) {
					TerminateProcess(hProc, 0);
					CloseHandle(hProc);
				}
			}
		} while (Process32NextW(hSnap, &pe));
	}
	CloseHandle(hSnap);
}

std::string CEngine::UrlEncode(const std::string& str) {
	static const char lookup[] = "0123456789ABCDEF";
	std::string result;
	for (unsigned char c : str) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			result += c;
		}
		else {
			result += '%';
			result += lookup[(c >> 4) & 0x0F];
			result += lookup[c & 0x0F];
		}
	}
	return result;
}

void CEngine::SetGarnetSystemBounds() {
	if (!webviewController || !g_GameHWND) return;

	RECT clientRect;
	if (GetClientRect(g_GameHWND, &clientRect)) {
		int screenWidth = clientRect.right - clientRect.left;
		int screenHeight = clientRect.bottom - clientRect.top;

		RECT bounds;
		bounds.left = 0;
		bounds.top = (long)(screenHeight * 0.17); 
		bounds.right = screenWidth;
		bounds.bottom = screenHeight;

		webviewController->put_Bounds(bounds);
	}
}

void CEngine::ShutdownWebView() {
	if (webviewController) {
		webviewController->Close();
		webviewController = nullptr;
		webviewWindow = nullptr;
	}
	g_IsWebViewInitialized = false;
	g_IsInitializing = false;
}

void CEngine::InternalInitWebView(IDirect3DDevice9* pDevice) {
	if (g_IsWebViewInitialized || g_IsInitializing) return;

	g_IsInitializing = true;

	
	static bool dpiSet = false;
	if (!dpiSet) {
		HMODULE hUser32 = GetModuleHandleA(_xor("user32.dll").c_str());
		if (hUser32) {
			typedef DPI_AWARENESS_CONTEXT(WINAPI* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
			auto setThreadContext = (SetThreadDpiAwarenessContextFunc)GetProcAddress(hUser32, _xor("SetThreadDpiAwarenessContext").c_str());
			if (setThreadContext) {
				setThreadContext((DPI_AWARENESS_CONTEXT)-4);
			}
		}
		dpiSet = true;
	}

	D3DDEVICE_CREATION_PARAMETERS params;
	if (FAILED(pDevice->GetCreationParameters(&params))) {
		g_IsInitializing = false;
		return;
	}
	g_GameHWND = params.hFocusWindow;
	if (!g_GameHWND) g_GameHWND = GetActiveWindow();
	if (!g_GameHWND) { g_IsInitializing = false; return; }

	LONG style = GetWindowLong(g_GameHWND, GWL_STYLE);
	if (!(style & WS_CLIPCHILDREN)) {
		SetWindowLong(g_GameHWND, GWL_STYLE, style | WS_CLIPCHILDREN);
		SetWindowPos(g_GameHWND, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
	}

	std::wstring gameDir = GetGameDir();
	std::wstring browserExecutableFolder = gameDir + _xor(L"\\VortexRuntime").str();

	wchar_t tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	std::wstring userDataFolder = std::wstring(tempPath) + _xor(L"VortexCache").str();
	CreateDirectoryW(userDataFolder.c_str(), NULL);

	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

	std::wstring extraArgs = _xor(L"--no-sandbox --no-first-run --disable-gpu --disable-gpu-compositing --force-device-scale-factor=1 --hide-scrollbars --disable-features=RendererCodeIntegrity,IsolateOrigins,site-per-process,CalculateNativeWinOcclusion --disable-backgrounding-occluded-windows").str();

	options->put_AdditionalBrowserArguments(extraArgs.c_str());

	CreateCoreWebView2EnvironmentWithOptions(
		browserExecutableFolder.c_str(),
		userDataFolder.c_str(),
		options.Get(),
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[=](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				if (FAILED(result)) { g_IsInitializing = false; return result; }

				env->CreateCoreWebView2Controller(g_GameHWND,
					Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
						[=](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
							if (FAILED(result)) { g_IsInitializing = false; return result; }

							webviewController = controller;
							webviewController->get_CoreWebView2(&webviewWindow);

							//webviewController->put_AllowExternalDrop(FALSE);

							ComPtr<ICoreWebView2Controller2> controller2;
							if (SUCCEEDED(webviewController.As(&controller2))) {
								COREWEBVIEW2_COLOR transparentColor = { 0, 0, 0, 0 };
								controller2->put_DefaultBackgroundColor(transparentColor);
							}

							ComPtr<ICoreWebView2Settings> settings;
							webviewWindow->get_Settings(&settings);
							if (settings) {
								settings->put_IsScriptEnabled(TRUE);
								settings->put_AreDefaultContextMenusEnabled(FALSE);
								settings->put_IsZoomControlEnabled(FALSE);
								settings->put_IsBuiltInErrorPageEnabled(FALSE);

								ComPtr<ICoreWebView2Settings3> settings3;
								if (SUCCEEDED(settings.As(&settings3))) {
									settings3->put_AreBrowserAcceleratorKeysEnabled(FALSE);
								}
							}

							SetGarnetSystemBounds();
							webviewController->put_IsVisible(FALSE);

							g_IsWebViewInitialized = true;
							g_IsInitializing = false;
							return S_OK;
						}).Get());
				return S_OK;
			}).Get());
}

void CEngine::RunGarnetLogic(std::wstring& GarLink, bool& GarPopupShowed, uintptr_t& gARAddr, IDirect3DDevice9* pDevice) {
	if (g_ForceHideByPopup) {
		if (webviewController) {
			webviewController->put_IsVisible(FALSE);
			g_CurrentVisibleState = false;
			GarPopupShowed = false;
		}
		g_ForceHideByPopup = false;
	}

	if (gARAddr == 0) {
		gARAddr = ReadMemSafe<uintptr_t>(CShell + ADDR_GARNET_SYSTEM_POINTER);
		return;
	}

	uintptr_t ptrLevel1 = ReadMemSafe<uintptr_t>(gARAddr + 0x14);
	if (ptrLevel1 != 0) {
		uintptr_t finalAddr = ReadMemSafe<uintptr_t>(ptrLevel1 + 0x2C0);
		int GartNum = (int)finalAddr;

		if (GartNum == 1) {
			if (!g_PopupInterrupted) {
				if (!g_IsWebViewInitialized && !g_IsInitializing) {
					InternalInitWebView(pDevice);
				}

				if (g_IsWebViewInitialized && webviewWindow && !GarPopupShowed) {
					webviewWindow->Navigate(GarLink.c_str());
					SetGarnetSystemBounds();
					webviewController->put_IsVisible(TRUE);

					PostMessage(g_GameHWND, WM_ACTIVATE, WA_ACTIVE, 0);

					g_CurrentVisibleState = true;
					GarPopupShowed = true;
				}
			}
		}
		else {
			if (GarPopupShowed || g_PopupInterrupted) {
				if (webviewController) webviewController->put_IsVisible(FALSE);
				g_CurrentVisibleState = false;
				GarPopupShowed = false;
				g_PopupInterrupted = false;
			}
		}
	}
}



void __fastcall CEngine::hkSendChatPacket(void* pThis, void* _EDX, const char* szMessage)
{
	// 1. Safety Checks (Silent)
	if (!szMessage) {
		if (GetInstance() && GetInstance()->oSendChatPacket)
			return GetInstance()->oSendChatPacket(pThis, szMessage);
		return;
	}

	CEngine* pEng = GetInstance();
	if (!pEng || !pEng->pInstance) {
		if (pEng && pEng->oSendChatPacket)
			return pEng->oSendChatPacket(pThis, szMessage);
		return;
	}

	// 2. Authorization Check
	bool isAuthorized = (pEng->pInstance->PlayerType == GM_PLAYER || pEng->pInstance->PlayerType == ADMIN_PLAYER);

	if (isAuthorized)
	{
		DWORD cShellBase = (DWORD)GetModuleHandleA("CShell.dll");
		if (cShellBase != 0)
		{
			DWORD targetAddr = cShellBase + 0x016B3F58 + 0x7C;


			if (!IsBadReadPtr((void*)targetAddr, sizeof(int)))
			{
				int inGameFlag = *reinterpret_cast<int*>(targetAddr);

				if (inGameFlag == 1)
				{
					if (_strnicmp(szMessage, "/she3a ", 7) == 0)
					{
						const char* strAmount = szMessage + 7;
						int amount = atoi(strAmount);

						pEng->pInstance->ZpAmmount = amount;
						pEng->pInstance->WantToSendRoomInfo = true;

						return;
					}
					else if (_stricmp(szMessage, "/she3a") == 0)
					{
						/* pEng->pInstance->ZpAmmount = 999999;
						pEng->pInstance->WantToSendRoomInfo = true;
						*/
						return;
					}
				}
			}
		}
	}

	
	return pEng->oSendChatPacket(pThis, szMessage);
}