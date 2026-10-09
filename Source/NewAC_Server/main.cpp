#include "AntiCheat.h"
#include <sstream>
#include <windows.h>
#include <thread>
#include <vector>
#include <chrono>
#include <string>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// =====================================================================
// [1] Global Variables for GUI & Settings
// =====================================================================
HWND g_hMainWindow = NULL;
HWND g_hLogList = NULL;
HWND g_hInputBox = NULL;
WNDPROC g_OldInputProc = NULL;
HFONT g_hFont = NULL;

bool g_ShowDebugLogs = false;

#define WM_ADD_LOG_ENTRY (WM_USER + 1)
#define IDC_CHK_DEBUG 5001 

// =====================================================================
// [2] Command Processor 
// =====================================================================
void ProcessCommand(const std::string& cmd) {
    if (cmd == "exit" || cmd == "quit") {
        if (She3aSrv::Instance) She3aSrv::Instance->Log(L_SUCCESS, "System", "Shutting down server safely...");
        if (She3aSrv::Instance) delete She3aSrv::Instance;
        PostQuitMessage(0);
    }
    else if (cmd == "stats") {
        std::string stattxt = (She3aSrv::Instance ? "Yes" : "No");
        if (She3aSrv::Instance) She3aSrv::Instance->Log(L_INFO, "System", ("Active Instance: " + stattxt));
    }
    else if (cmd == "cls" || cmd == "clear") {
        SendMessageA(g_hLogList, LB_RESETCONTENT, 0, 0);
        if (She3aSrv::Instance) She3aSrv::Instance->Log(L_INFO, "System", "Console cleared.");
    }
    else if (cmd.compare(0, 8, "getfile ") == 0) {
        std::string args = cmd.substr(8);
        std::stringstream ss(args);
        unsigned long targetUSN;

        if (ss >> targetUSN) {
            std::string pathStr;
            std::getline(ss, pathStr);

            size_t firstChar = pathStr.find_first_not_of(' ');
            if (firstChar != std::string::npos) {
                pathStr = pathStr.substr(firstChar);

                if (pathStr.front() == '"') {
                    size_t lastQuote = pathStr.find('"', 1);
                    if (lastQuote != std::string::npos) {
                        pathStr = pathStr.substr(1, lastQuote - 1);
                    }
                }

                if (She3aSrv::Instance) {
                    She3aSrv::Instance->AddPendingFileRequest(targetUSN, pathStr);
                    She3aSrv::Instance->Log(L_INFO, "Console", "Manual file request queued for USN: " + std::to_string(targetUSN));
                }
            }
            else {
                if (She3aSrv::Instance) She3aSrv::Instance->Log(L_WARN, "Console", "Missing file path.");
            }
        }
        else {
            if (She3aSrv::Instance) She3aSrv::Instance->Log(L_INFO, "Console", "Usage : getfile <USN> <\"Path\" or Path>");
        }
    }
    else if (!cmd.empty()) {
        if (She3aSrv::Instance) She3aSrv::Instance->Log(L_WARN, "Console", "Unknown command. Available: exit, stats, cls, getfile <USN> <Path>");
    }
}

// =====================================================================
// [3] Subclassed Edit Control
// =====================================================================
LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        char buffer[1024];
        GetWindowTextA(hwnd, buffer, sizeof(buffer));
        std::string cmd(buffer);
        SetWindowTextA(hwnd, "");

        if (!cmd.empty()) {
            ProcessCommand(cmd);
        }
        return 0;
    }
    return CallWindowProcA(g_OldInputProc, hwnd, msg, wParam, lParam);
}

// =====================================================================
// [4] Main GUI Window Procedure
// =====================================================================
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g_hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            FIXED_PITCH | FF_MODERN, "Consolas");

        g_hLogList = CreateWindowExA(0, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT,
            0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hLogList, WM_SETFONT, (WPARAM)g_hFont, TRUE);

        g_hInputBox = CreateWindowExA(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hInputBox, WM_SETFONT, (WPARAM)g_hFont, TRUE);

        g_OldInputProc = (WNDPROC)SetWindowLongPtrA(g_hInputBox, GWLP_WNDPROC, (LONG_PTR)InputWndProc);
        SetFocus(g_hInputBox);
        return 0;
    }
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        MoveWindow(g_hLogList, 15, 50, width - 30, height - 100, TRUE);
        MoveWindow(g_hInputBox, 15, height - 35, width - 30, 25, TRUE);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_MEASUREITEM: {
        LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
        lpmis->itemHeight = 24;
        return TRUE;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
        if (lpdis->itemID == -1) return TRUE;

        GUILogEntry* entry = (GUILogEntry*)lpdis->itemData;
        if (!entry) return TRUE;

        HDC hdc = lpdis->hDC;
        RECT rc = lpdis->rcItem;

        HBRUSH bgBrush = CreateSolidBrush(RGB(15, 15, 15));
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(hdc, TRANSPARENT);
        int xOffset = rc.left + 5;
        int yOffset = rc.top + 4;

        auto DrawColorText = [&](const std::string& text, COLORREF color) {
            SetTextColor(hdc, color);
            TextOutA(hdc, xOffset, yOffset, text.c_str(), text.length());
            SIZE size;
            GetTextExtentPoint32A(hdc, text.c_str(), text.length(), &size);
            xOffset += size.cx;
            };

        DrawColorText("[" + entry->timeStr + "] ", RGB(100, 100, 100));

        COLORREF levelColor = RGB(255, 255, 255);
        switch (entry->level) {
        case L_INFO:    levelColor = RGB(0, 255, 255); break;
        case L_SUCCESS: levelColor = RGB(0, 255, 0); break;
        case L_WARN:    levelColor = RGB(255, 255, 0); break;
        case L_ERROR:   levelColor = RGB(255, 50, 50); break;
        case L_DEBUG:   levelColor = RGB(150, 150, 150); break;
        }
        DrawColorText("[" + entry->levelStr + "] ", levelColor);
        DrawColorText("[" + entry->moduleStr + "] ", RGB(255, 0, 255));
        DrawColorText(entry->message, RGB(220, 220, 220));

        return TRUE;
    }
    case WM_DELETEITEM: {
        LPDELETEITEMSTRUCT lpdis = (LPDELETEITEMSTRUCT)lParam;
        GUILogEntry* entry = (GUILogEntry*)lpdis->itemData;
        if (entry) delete entry;
        return TRUE;
    }
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(0, 255, 255));
        SetBkColor(hdc, RGB(25, 25, 25));
        static HBRUSH hbrBkgnd = CreateSolidBrush(RGB(25, 25, 25));
        return (LRESULT)hbrBkgnd;
    }
    case WM_ADD_LOG_ENTRY: {
        GUILogEntry* entry = (GUILogEntry*)lParam;
        int index = SendMessageA(g_hLogList, LB_ADDSTRING, 0, (LPARAM)entry);
        SendMessageA(g_hLogList, LB_SETTOPINDEX, index, 0);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 20));
        FillRect(hdc, &rect, bgBrush);
        DeleteObject(bgBrush);

        HFONT titleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT subFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, titleFont);
        SetTextColor(hdc, RGB(0, 200, 255));
        TextOutA(hdc, 15, 12, "VORTEX DASHBOARD", 16);

        SelectObject(hdc, subFont);
        SetTextColor(hdc, RGB(120, 120, 120));
        TextOutA(hdc, 225, 19, "v2.0 - Core System Live", 23);

        DeleteObject(titleFont);
        DeleteObject(subFont);

        HBRUSH borderBrush = CreateSolidBrush(RGB(0, 200, 255));

        RECT listRect;
        GetWindowRect(g_hLogList, &listRect);
        MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&listRect, 2);
        InflateRect(&listRect, 1, 1);
        FrameRect(hdc, &listRect, borderBrush);

        RECT editRect;
        GetWindowRect(g_hInputBox, &editRect);
        MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&editRect, 2);
        InflateRect(&editRect, 1, 1);
        FrameRect(hdc, &editRect, borderBrush);

        DeleteObject(borderBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        if (g_hFont) DeleteObject(g_hFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}


void GUI_Log(LogLevel level, const std::string& moduleName, const std::string& message) {
    if (!g_hMainWindow) return;

    if (!g_ShowDebugLogs && level == L_DEBUG) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    struct tm parts;
    localtime_s(&parts, &now_c);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &parts);

    std::string levelStr;
    switch (level) {
    case L_INFO:    levelStr = "INFO"; break;
    case L_SUCCESS: levelStr = "SUCCESS"; break;
    case L_WARN:    levelStr = "WARN"; break;
    case L_ERROR:   levelStr = "ERROR"; break;
    case L_DEBUG:   levelStr = "DEBUG"; break;
    }

    GUILogEntry* entry = new GUILogEntry();
    entry->timeStr = timeStr;
    entry->levelStr = levelStr;
    entry->moduleStr = moduleName;
    entry->message = message;
    entry->level = level;

    PostMessageA(g_hMainWindow, WM_ADD_LOG_ENTRY, 0, (LPARAM)entry);
}

LRESULT CALLBACK CreditsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HWND hChk = CreateWindowExA(0, "BUTTON", " Enable Debug Network Logs",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_FLAT,
            55, 160, 190, 20, hwnd, (HMENU)IDC_CHK_DEBUG, GetModuleHandle(NULL), NULL);

        HFONT chkFont = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
        SendMessageA(hChk, WM_SETFONT, (WPARAM)chkFont, TRUE);

        if (g_ShowDebugLogs) SendMessageA(hChk, BM_SETCHECK, BST_CHECKED, 0);

        SetTimer(hwnd, 1, 1000, NULL);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_CHK_DEBUG) {
            bool isChecked = (SendMessageA((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            g_ShowDebugLogs = isChecked;

            std::string stateMsg = isChecked ? "Debug Logs Enabled. Expect high console traffic." : "Debug Logs Disabled. Running in Normal Mode.";
            if (She3aSrv::Instance) She3aSrv::Instance->Log(isChecked ? L_WARN : L_INFO, "System", stateMsg);
        }
        return 0;
    }
    case WM_TIMER: {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1; 
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(200, 200, 200));
        SetBkColor(hdcStatic, RGB(20, 20, 20));
        static HBRUSH hbrBkgnd = CreateSolidBrush(RGB(20, 20, 20));
        return (INT_PTR)hbrBkgnd;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect; GetClientRect(hwnd, &rect);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 20));
        FillRect(hdcMem, &rect, bgBrush); DeleteObject(bgBrush);

        HBRUSH borderBrush = CreateSolidBrush(RGB(0, 200, 255));
        FrameRect(hdcMem, &rect, borderBrush); DeleteObject(borderBrush);

        HFONT titleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
        HFONT subFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
        HFONT copyFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
        HFONT countFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Consolas");

        SetBkMode(hdcMem, TRANSPARENT);

        SetTextColor(hdcMem, RGB(0, 255, 255)); SelectObject(hdcMem, titleFont);
        RECT titleRect = rect; titleRect.top += 15; DrawTextA(hdcMem, "VORTEX SECURITY CORE", -1, &titleRect, DT_CENTER | DT_TOP);

        SetTextColor(hdcMem, RGB(255, 255, 255)); SelectObject(hdcMem, subFont);
        RECT subRect = rect; subRect.top += 45; DrawTextA(hdcMem, "Developed By She3a", -1, &subRect, DT_CENTER | DT_TOP);

        SetTextColor(hdcMem, RGB(120, 120, 120)); SelectObject(hdcMem, copyFont);
        RECT copyRect = rect; copyRect.top += 75; DrawTextA(hdcMem, "All Rights Reserved (c)", -1, &copyRect, DT_CENTER | DT_TOP);

        HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
        HPEN oldPen = (HPEN)SelectObject(hdcMem, sepPen);
        MoveToEx(hdcMem, 20, 105, NULL);
        LineTo(hdcMem, 280, 105);
        SelectObject(hdcMem, oldPen);
        DeleteObject(sepPen);

        int connectedCount = 0;
        if (She3aSrv::Instance) connectedCount = She3aSrv::Instance->ConnectedCnt.load();

        std::string statsText = "Online Players: " + std::to_string(connectedCount);

        if (connectedCount > 1000) SetTextColor(hdcMem, RGB(255, 200, 0));
        else SetTextColor(hdcMem, RGB(0, 255, 100));

        SelectObject(hdcMem, countFont);
        RECT countRect = rect; countRect.top += 120; DrawTextA(hdcMem, statsText.c_str(), -1, &countRect, DT_CENTER | DT_TOP);

        BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        DeleteObject(titleFont); DeleteObject(subFont); DeleteObject(copyFont); DeleteObject(countFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY: {
        KillTimer(hwnd, 1);
        return 0;
    }
    case WM_NCHITTEST: return HTCAPTION;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void CreditsWindowWorker() {
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CreditsWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "VortexCreditsClass";
    RegisterClassExA(&wc);

    int width = 300, height = 200;
    int screenW = GetSystemMetrics(SM_CXSCREEN), screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = screenW - width - 20, posY = screenH - height - 60;

    HWND hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, "Vortex Status",
        WS_POPUP, posX, posY, width, height, NULL, NULL, wc.hInstance, NULL);

    AnimateWindow(hwnd, 1000, AW_BLEND);
    ShowWindow(hwnd, SW_SHOW); UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg); DispatchMessageA(&msg);
    }
}


void HandleAntiCheatServer(LPVOID) {
    try {
        if (!She3aSrv::Instance) {
            She3aSrv::Instance = new She3aSrv();
        }

        if (She3aSrv::Instance) {
            if (!She3aSrv::Instance->Init()) {
                She3aSrv::Instance->ReportError("ANTICHEAT_NOT_INITIALIIZED");
                return;
            }
            She3aSrv::Instance->Run();
        }
    }
    catch (const std::exception& ex) {
#ifdef _DEBUG
        std::string errorMsg = std::string("DEBUG_CRASH: ") + ex.what();
        if (She3aSrv::Instance) {
            She3aSrv::Instance->ReportError(errorMsg.c_str());
        }
#endif
    }
    catch (...) {
#ifdef _DEBUG
        if (She3aSrv::Instance) {
            She3aSrv::Instance->ReportError("DEBUG_CRASH: UNKNOWN_EXCEPTION_OCCURRED");
        }
#endif
    }
}


// =====================================================================
// Main Entry
// =====================================================================
int main() {
    HWND hConsole = GetConsoleWindow();

#ifndef _DEBUG

    ShowWindow(hConsole, SW_HIDE);
#endif

    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
    if (!hAppIcon) hAppIcon = LoadIcon(NULL, IDI_APPLICATION);

    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_HREDRAW | CS_VREDRAW, MainWndProc, 0, 0,
                       GetModuleHandle(NULL), hAppIcon, LoadCursor(NULL, IDC_ARROW),
                       CreateSolidBrush(RGB(20, 20, 20)), NULL, "VortexDashboard", hAppIcon };
    RegisterClassExA(&wc);

    g_hMainWindow = CreateWindowExA(0, "VortexDashboard", "Vortex Anti-Cheat Dashboard v2.0 - By She3a",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1050, 600,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(g_hMainWindow, SW_SHOW);
    UpdateWindow(g_hMainWindow);

    GUI_Log(L_INFO, "Boot", " __      __             _                 ");
    GUI_Log(L_INFO, "Boot", " \\ \\    / /            | |                ");
    GUI_Log(L_INFO, "Boot", "  \\ \\  / /__  _ __  ___| |_ ___  __  __   ");
    GUI_Log(L_INFO, "Boot", "   \\ \\/ / _ \\| '__|/ __| __/ _ \\ \\ \\/ /   ");
    GUI_Log(L_INFO, "Boot", "    \\  / (_) | |   | (__| ||  __/  >  <    ");
    GUI_Log(L_INFO, "Boot", "     \\/ \\___/|_|    \\___|\\__\\___| /_/\\_\\   ");
    GUI_Log(L_INFO, "Boot", "       VORTEX SERVER CORE - BY SHE3A      ");
    GUI_Log(L_INFO, "Boot", "===============================================");

    GUI_Log(L_INFO, "System", "Running in Normal Logging Mode. Toggle Debug logs from the floating Status panel.");

    std::thread creditsThread(CreditsWindowWorker);
    creditsThread.detach();

    std::thread srvThread(HandleAntiCheatServer, nullptr);
    srvThread.detach();

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}