#include "HookMethodInDll.h"

FARPROC g_methodAddress = NULL;
LPVOID g_savedFirstInstructionOfMethodMemoryBeforeOverwriting;
const int FIRST_INSTRUCTION_SIZE = 5;

HWND __stdcall HideTeamsControlBar_stdcall(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

HWND __stdcall HideTeamsControlBar_stdcall(DWORD dwExStyle,
	LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
	int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	MessageBoxA(hWndParent, "hehe", "we reached the method leleell", 0);

	// unhook the function (re-write the saved buffer) to prevent infinite recursion
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)g_methodAddress, g_savedFirstInstructionOfMethodMemoryBeforeOverwriting, FIRST_INSTRUCTION_SIZE, NULL);

	// return to the original function and modify the intended parameters TODO
	return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HookMethodInDll::HookMethodInDll(std::string targetProcessName, std::string targetDLLName, std::string targetMethodName)
{
    m_targetProcessName = targetProcessName;
	m_targetMethodName = targetMethodName;
	m_targetDLLName = targetDLLName;

	if (!AttachProcess()) throw std::exception("Process is not running");

	m_targetDll = GetModule(targetDLLName);
	m_targetDLL_Base = (DWORD)m_targetDll.modBaseAddr;
	if (m_targetDLL_Base == 0x0) throw std::exception("Dll base is NULL, meaning the dll wasn't loaded properly");

	g_savedFirstInstructionOfMethodMemoryBeforeOverwriting = new char[FIRST_INSTRUCTION_SIZE];

	InstallHookOnMethod();
}

HookMethodInDll::~HookMethodInDll()
{
	// Unhook the function (re-write the saved buffer) to prevent infinite recursion
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)g_methodAddress, g_savedFirstInstructionOfMethodMemoryBeforeOverwriting, FIRST_INSTRUCTION_SIZE, NULL);

	CloseHandle(m_hProcess);
	delete[FIRST_INSTRUCTION_SIZE] g_savedFirstInstructionOfMethodMemoryBeforeOverwriting;
}

void HookMethodInDll::InstallHookOnMethod()
{
	HINSTANCE hinstLib;
	VOID* proxy_address;
	DWORD* relative_offset;
	DWORD src;
	DWORD dst;
	CHAR patch[5] = { 0 };

	// 1. get memory address of the method from the dll
	hinstLib = LoadLibraryA(m_targetDLLName.c_str());
	if (hinstLib == NULL)
	{
		throw std::exception("Could not get the dll inside the target process");
	}

	g_methodAddress = GetProcAddress(hinstLib, m_targetMethodName.c_str());

	// 2. save the first 5 bytes into saved_buffer
	ReadProcessMemory(m_hProcess, g_methodAddress, g_savedFirstInstructionOfMethodMemoryBeforeOverwriting, FIRST_INSTRUCTION_SIZE, NULL);

	// 3. overwrite the first 5 bytes with a call to proxy_function
	proxy_address = &HideTeamsControlBar_stdcall;
	src = (DWORD)g_methodAddress + FIRST_INSTRUCTION_SIZE;
	dst = (DWORD)proxy_address;
	relative_offset = (DWORD*)(dst - src);
	memcpy_s(patch, 1, "\xE9", 1); // xE9 is jmp, 1 for first byte
	memcpy_s(patch + 1, FIRST_INSTRUCTION_SIZE - 1, &relative_offset, FIRST_INSTRUCTION_SIZE - 1);

	WriteProcessMemory(m_hProcess, (LPVOID)g_methodAddress, patch, FIRST_INSTRUCTION_SIZE, NULL);
}

bool HookMethodInDll::AttachProcess()
{
	HANDLE hPid = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	int nChars = MultiByteToWideChar(CP_ACP, 0, m_targetProcessName.c_str(), -1, NULL, 0);
	const WCHAR* procNameChar = new WCHAR[nChars];

	MultiByteToWideChar(CP_ACP, 0, m_targetProcessName.c_str(), -1, (LPWSTR)procNameChar, nChars);
	char* processName = new char[nChars];
	wcstombs(processName, procNameChar, nChars);

	do //finds the correct process in the procEntry with the handler process id
	{
		if (wcscmp(procEntry.szExeFile, procNameChar) == 0)
		{
			m_pid = procEntry.th32ProcessID;
			CloseHandle(hPid);
			m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
			return true;
		}
	} while (Process32Next(hPid, &procEntry));

	delete[nChars] procNameChar;
	CloseHandle(hPid);
	return false;
}

MODULEENTRY32 HookMethodInDll::GetModule(std::string searchedModuleName)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);

	const WCHAR* modNameChar;
	int nChars = MultiByteToWideChar(CP_ACP, 0, searchedModuleName.c_str(), -1, NULL, 0);
	modNameChar = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, searchedModuleName.c_str(), -1, (LPWSTR)modNameChar, nChars);
	char* moduleName = new char[nChars];
	wcstombs(moduleName, modNameChar, nChars);
	do
	{
		if (wcscmp(mEntry.szModule, modNameChar) == 0)
		{
			CloseHandle(hModule);
			return mEntry;
		}
	} while (Module32Next(hModule, &mEntry));

	CloseHandle(hModule);
	delete[nChars] moduleName;
	mEntry.modBaseAddr = 0x0;
	return mEntry;
}