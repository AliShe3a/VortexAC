#include "stdafx.h"
#include "SAC.h"
#include <Psapi.h>
#include <Shlwapi.h>
#include <chrono>
#pragma comment(lib, "version.lib")

static int g_TotalOverlayDetections = 0;
struct WhitelistEntry {
    std::string fileName;
    std::string publisherName;
    std::string serialCode;
};
bool VerifyInternalName(const char* path, const char* expectedName) {
    DWORD dummy;
    DWORD size = GetFileVersionInfoSizeA(path, &dummy);
    if (size == 0) return false;
    std::vector<char> buffer(size);
    if (!GetFileVersionInfoA(path, 0, size, buffer.data())) return false;
    LPSTR originalName = NULL;
    UINT nameLen = 0;
    if (VerQueryValueA(buffer.data(), _xor("\\StringFileInfo\\040904b0\\OriginalFilename").c_str(), (LPVOID*)&originalName, &nameLen)) {
        if (originalName && _stricmp(originalName, expectedName) == 0) return true;
    }
    return false;
}

OverlayFileInfo She3aAC::GetOverlayFileInfo(const char* processPath, const char* className, const char* windowTitle) {
    OverlayFileInfo info;
    info.isSigned = IsModuleSigned(processPath);
    info.isWhitelisted = false;
    info.className = className;
    info.fileName = PathFindFileNameA(processPath);

    std::string pathLower = processPath;
    for (auto& c : pathLower) c = (char)tolower(c);

    if (info.isSigned) {
        info.serialCode = GetCertSerialCode(processPath);
        info.publisherName = GetModulePublisherNameSafe(processPath);

        static const WhitelistEntry trustedApps[] = {
            { _xor("explorer.exe").c_str(), _xor("Microsoft Windows").c_str(), _xor("A7 04 00 00 00 00 FC FA 34 C8 22 E4 3E 04 A7 04 00 00 33").c_str() },
            { _xor("obs64.exe").c_str(), _xor("OBS Project, LLC").c_str(), _xor("37 AB 4D C5 EE DE E8 DE 91 C1 B8 83 06 6A 41 0D").c_str() },
            { _xor("Discord.exe").c_str(), _xor("Discord Inc.").c_str(), _xor("D7 34 3E 09 83 0D 2E 06 A0 64 83 71 2E CF E9 0D").c_str() },
            { _xor("MSIAfterburner.exe").c_str(), _xor("MICRO-STAR INTERNATIONAL CO., LTD.").c_str(), _xor("D6 5A 9D 70 88 82 4F 3E E8 21 5E 08").c_str() }
        };

        for (const auto& app : trustedApps) {
            if (info.fileName == app.fileName &&
                info.publisherName == app.publisherName &&
                info.serialCode == app.serialCode)
            {
                info.isWhitelisted = true;
                return info;
            }
        }

        if (info.fileName == _xor("crossfire.exe").c_str() && info.publisherName.find(_xor("Smilegate").c_str()) != std::string::npos) {
            if (std::string(className) == _xor("#32770").c_str() && std::string(windowTitle) == _xor("CF EPIC ERROR").c_str()) {
                info.isWhitelisted = true;
            }
        }
    }
    else {
        info.serialCode = _xor("N/A").c_str();
        info.publisherName = _xor("Unsigned Content").c_str();
    }

    if (std::string(className) == _xor("ConsoleWindowClass").c_str() && info.fileName == _xor("cmd.exe").c_str()) {
        if ((pathLower.find(_xor("\\system32\\").c_str()) != std::string::npos) && info.isSigned && info.publisherName.find(_xor("Microsoft").c_str()) != std::string::npos) {
            if (VerifyInternalName(processPath, _xor("cmd.exe").c_str())) {
                info.isWhitelisted = true;
            }
        }
    }

    return info;
}

HWND GetGameWindowHandle(DWORD processId) {
    struct HandleData { DWORD pid; HWND hwnd; };
    HandleData data = { processId, NULL };
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        HandleData* pData = (HandleData*)lParam;
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (pData->pid != windowPid || !IsWindowVisible(hwnd)) return TRUE;
        char className[256], windowTitle[256];
        GetClassNameA(hwnd, className, sizeof(className));
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
        if (strcmp(className, _xor("CrossFire").c_str()) == 0 && strcmp(windowTitle, _xor("CFEPICGAMING").c_str()) == 0) {
            pData->hwnd = hwnd;
            return FALSE;
        }
        return TRUE;
        }, (LPARAM)&data);
    return data.hwnd;
}

bool She3aAC::IsExternalOverlayDetected() {
    static HWND staticGameHwnd = NULL;
    if (staticGameHwnd == NULL || !IsWindow(staticGameHwnd)) {
        staticGameHwnd = GetGameWindowHandle(GetCurrentProcessId());
    }

    if (!staticGameHwnd) return false;

    bool isFocused = (GetForegroundWindow() == staticGameHwnd);
    RECT gameRect;
    GetWindowRect(staticGameHwnd, &gameRect);

    ScanContext context;
    context.hGame = staticGameHwnd;
    context.rGame = gameRect;
    context.pGame = GetCurrentProcessId();
    context.pAC = this;
    context.isFocused = isFocused;
    context.gameFoundInZOrder = false;
    context.detected = false;

    for (auto& pair : m_TrackedOverlays) pair.second.isActive = false;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        ScanContext* pCtx = (ScanContext*)lParam;

        if (hwnd == pCtx->hGame) {
            pCtx->gameFoundInZOrder = true;
            return TRUE;
        }

        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (!IsWindowVisible(hwnd)) return TRUE;

        if (windowPid == pCtx->pGame) {
            pCtx->detected = true;
            /*printf("\n[!!!] INTERNAL HIJACK DETECTED [!!!]\n");
            printf("> Window PID: %d (Same as game)\n", windowPid);*/
            pCtx->pAC->cPlayer->BanMsg = _xor("INTERNAL OVERLAY DETECTED!").c_str();
            SetWindowDisplayAffinity(hwnd, 0);
            ShowWindow(hwnd, SW_SHOW);
            return FALSE;
        }

        if (!pCtx->gameFoundInZOrder && pCtx->isFocused) {
            RECT winRect;
            GetWindowRect(hwnd, &winRect);
            RECT intersection;

            if (IntersectRect(&intersection, &pCtx->rGame, &winRect)) {
                LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                bool isOverlayStyle = (exStyle & WS_EX_TOPMOST) || (exStyle & WS_EX_LAYERED) || (exStyle & WS_EX_TRANSPARENT);

                if (isOverlayStyle) {
                    auto& tracker = pCtx->pAC->m_TrackedOverlays[hwnd];
                    if (tracker.firstSeen.time_since_epoch().count() == 0) {
                        tracker.firstSeen = std::chrono::steady_clock::now();
                    }
                    tracker.isActive = true;

                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - tracker.firstSeen).count();

                    if (duration > 2500) {
                        char processPath[MAX_PATH] = "Unknown";
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, windowPid);
                        if (hProcess) {
                            GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH);
                            CloseHandle(hProcess);
                        }

                        char wClass[256], wTitle[256];
                        GetClassNameA(hwnd, wClass, sizeof(wClass));
                        GetWindowTextA(hwnd, wTitle, sizeof(wTitle));

                        OverlayFileInfo info = pCtx->pAC->GetOverlayFileInfo(processPath, wClass, wTitle);

                        if (!info.isWhitelisted) {
                            pCtx->detected = true;
                            pCtx->pAC->cPlayer->BanMsg = std::string(_xor("EXTERNAL OVERLAY DETECTED [").c_str())
                                + info.fileName
                                + _xor("] [").c_str()
                                + std::to_string(duration)
                                + _xor(" ms]").c_str();
                            g_TotalOverlayDetections++;

                            SetWindowDisplayAffinity(hwnd, 0);
                            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~(WS_EX_TRANSPARENT | WS_EX_LAYERED));
                            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
                            ShowWindow(hwnd, SW_SHOW);

                            //printf("\n[!!!] EXTERNAL OVERLAY DETECTED [#%d] [!!!]\n", g_TotalOverlayDetections);
                            //printf("> Window: %s | Class: %s\n", wTitle[0] ? wTitle : "N/A", wClass);
                            //printf("> EXE: %s | Duration Above: %lld ms\n", info.fileName.c_str(), duration);
                            //printf("> Context: Z-ORDER ABOVE & FOCUSED\n");
                            //printf("------------------------------------\n");
                            return FALSE;
                        }
                    }
                }
            }
        }
        return TRUE;
        }, (LPARAM)&context);

    for (auto it = m_TrackedOverlays.begin(); it != m_TrackedOverlays.end(); ) {
        if (!it->second.isActive) it = m_TrackedOverlays.erase(it);
        else ++it;
    }



    return context.detected;
}