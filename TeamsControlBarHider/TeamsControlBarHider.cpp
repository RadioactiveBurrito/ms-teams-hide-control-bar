#include "HookMethodInDll.h"

#include <iostream>
#include <winnt.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

using namespace std;

const wchar_t* TEAMS_PROCESS_IMAGE_NAME = L"Teams.exe";
const string TEAMS_SCREENSHARE_CONTROL_BAR_NAME = "Screen sharing toolbar";

DWORD FindProcessByImageName(const wchar_t* procname) {

    HANDLE hSnapshot;
    PROCESSENTRY32 pe;
    int pid = 0;
    BOOL hResult;

    // snapshot of all processes in the system
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

    // initializing size: needed for using Process32First
    pe.dwSize = sizeof(PROCESSENTRY32);

    // info about first process encountered in a system snapshot
    hResult = Process32First(hSnapshot, &pe);

    // retrieve information about the processes
    // and exit if unsuccessful
    while (hResult) {
        // if we find the process: return process ID
        if (wcscmp(procname, pe.szExeFile) == 0) {
            pid = pe.th32ProcessID;
            break;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }

    // closes an open handle (CreateToolhelp32Snapshot)
    CloseHandle(hSnapshot);
    return pid;
}

HWND ControlBarWindow = NULL;
BOOL CALLBACK HideTeamsControlBar(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        ControlBarWindow = hwnd;
        ShowWindow(ControlBarWindow, SW_HIDE); // TODO: CHECK IF I COULD CLOSE IT INSTEAD
        return FALSE;
    }
    return TRUE;
}

// TODO: MAKE A WINDOWS SERVICE
// TODO: HOOK WINDOW OPEN TO CHECK IF THAT WINDOW HAS THE APPROPRIATE NAME
int main()
{
    // Tried with user32.dll, doesnt work
    HookMethodInDll hook = HookMethodInDll("Teams.exe", "ntdll.dll", "CreateWindowExA");

    //DWORD teamsProcessId = FindProcessByImageName(TEAMS_PROCESS_IMAGE_NAME);
    //// This works like a charm
    //while (true)
    //{
    //    Sleep(10);
    //    EnumWindows(HideTeamsControlBar, teamsProcessId);
    //}
    Sleep(60000);
}