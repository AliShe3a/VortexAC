#pragma once
#include <TlHelp32.h>
char Path[MAX_PATH];
char Path2[MAX_PATH];
DWORD64 exe = 0;
std::uint64_t find_process(const std::string& name);
int inject();
using namespace std;

const char* dllFile = (_xor("discord_rpc.dll").c_str());
const char* dllFile2 = (_xor("user_rpc.dll").c_str());
const char* targetProcess = (_xor("crossfire.exe").c_str());

int inject()
{
    // Get full paths for both DLLs
    GetFullPathNameA(_xor("discord_rpc.dll").c_str(), MAX_PATH, Path, NULL);
    GetFullPathNameA(_xor("user_rpc.dll").c_str(), MAX_PATH, Path2, NULL);

    // Find the target process (crossfire.exe)
    exe = find_process(_xor("crossfire.exe").c_str());

    // Open the target process with all access
    auto const acc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, exe);

    if (acc && acc != INVALID_HANDLE_VALUE)
    {
        // Get NtOpenFile function address
        const LPVOID Alloc = GetProcAddress(LoadLibraryW(_xor(L"ntdll").c_str()), _xor("NtOpenFile").c_str());
        if (Alloc)
        {
            char byte[5];
            memcpy(byte, Alloc, 5);
            WriteProcessMemory(acc, Alloc, byte, 5, nullptr);
        }

        // Allocate memory for both DLL paths in the remote process
        auto* alls1 = VirtualAllocEx(acc, nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        WriteProcessMemory(acc, alls1, Path, strlen(Path) + 1, nullptr);

        // Create remote thread to load the first DLL (discord_rpc.dll)
        auto* const thread1 = CreateRemoteThread(acc, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), alls1, 0, nullptr);
        if (thread1) CloseHandle(thread1);

        // Allocate memory for the second DLL path
        auto* alls2 = VirtualAllocEx(acc, nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        WriteProcessMemory(acc, alls2, Path2, strlen(Path2) + 1, nullptr);

        // Create remote thread to load the second DLL (user_rpc.dll)
        auto* const thread2 = CreateRemoteThread(acc, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), alls2, 0, nullptr);
        if (thread2) CloseHandle(thread2);
    }

    // Close the process handle
    if (acc) CloseHandle(acc);

    return 0;
}


std::uint64_t find_process(const std::string& name)
{
    // Convert the input ANSI string to a wide-character string
    std::wstring wname(name.begin(), name.end());

    const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 proc_entry{};
    proc_entry.dwSize = sizeof proc_entry;

    auto found_process = false;
    if (!!Process32First(snap, &proc_entry)) {
        do {
            // Convert the process name to wide-character for comparison
            std::wstring wproc_name(proc_entry.szExeFile);

            if (wname == wproc_name) {
                found_process = true;
                break;
            }
        } while (!!Process32Next(snap, &proc_entry));
    }

    CloseHandle(snap);
    return found_process ? proc_entry.th32ProcessID : 0;
}

void Injection()
{
    while (true)
    {
        Sleep(100);
        DWORD64 test = find_process(_xor("crossfire.exe").c_str());

        if (!test)
        {
            break;
        }
        else
        {
            inject();
            break;
        }
    }
    return;
}