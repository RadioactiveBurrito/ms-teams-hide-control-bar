#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <CommCtrl.h>
#include <shellapi.h>

using namespace std;

const LPWSTR TEAMS_SCREENSHARE_CONTROL_BAR_NAME = LPWSTR(L"Screen sharing toolbar");

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND ControlBarWindow = NULL;
BOOL CALLBACK HideTeamsControlBar(HWND hwnd, LPARAM lParam)
{
    std::wstring windowTitle(MAX_PATH, L'\0');
    GetWindowText(hwnd, (LPWSTR)windowTitle.c_str(), 256);
    if (wcscmp((LPWSTR)windowTitle.c_str(), (LPWSTR)lParam) == 0)
    {
        ControlBarWindow = hwnd;
        return ::SendMessage(hwnd, WM_CLOSE, NULL, NULL);
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_USER + 1:
        switch (lParam) {
        case WM_LBUTTONUP:
            // Code to run when the tray icon is left-clicked
            break;

        case WM_RBUTTONUP:
            // Code to run when the tray icon is right-clicked
            break;

        case WM_COMMAND:
        case WM_CREATE:
        case WM_DESTROY:
            MessageBoxA(NULL, "Tray icon clicked", "My Application", MB_OK);
            break;

        case WM_NOTIFY:
            // Handle tray notification messages
            switch (reinterpret_cast<LPNMHDR>(lParam)->code)
            {
            case NM_CLICK:
            case NM_RCLICK:
                // Tray was clicked or right-clicked, but don't remove the icon
                return TRUE;
            }
            break;

        default:
            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

LPWSTR ShowIconId;
HICON ShowIcon;
LPWSTR HideIconId;
HICON HideIcon;
void CreateIcons()
{
    ShowIcon = (HICON)LoadImage(NULL, L"Show.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED);
    ShowIconId = MAKEINTRESOURCE(ShowIcon);
    HideIcon = (HICON)LoadImage(NULL, L"Hide.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED);
    HideIconId = MAKEINTRESOURCE(HideIcon);
}

bool IsControlBarHidden = true;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    NOTIFYICONDATAA nid = {};


    //AllocConsole();
    //HWND h = FindWindowA("ConsoleWindowClass", NULL); //GetStdHandle(STD_OUTPUT_HANDLE);

    // Create a window with the specified window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;

    CreateIcons();

    wc.hIcon = LoadIcon(hInstance, ShowIconId);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"TeamsControlBarHiderClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowA("Teams Control Bar Hider", "Teams Control Bar Hider", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_SHOWTIP | NIF_MESSAGE;
    nid.hIcon = ShowIcon;
    nid.uCallbackMessage = WM_USER + 1;

    memcpy(nid.szTip, "Control Bar Visible", 128);
    memcpy(nid.szInfoTitle, "This is the info title", 64);
    memcpy(nid.szInfo, "This is the info itself", 256);
    //nid.dwInfoFlags =

    Shell_NotifyIconA(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIconA(NIM_DELETE, &nid);


    //while (true)
    //{
    //    EnumWindows(HideTeamsControlBar, (LPARAM)TEAMS_SCREENSHARE_CONTROL_BAR_NAME);
    //    Sleep(100);
    //}

    return msg.wParam;
}
