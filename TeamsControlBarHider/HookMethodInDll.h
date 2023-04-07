#pragma once
#pragma warning(disable : 4996)
#ifndef HOOK_METHOD_IN_DLL
#define HOOK_METHOD_IN_DLL

#include <iostream>
#include <windows.h> // Windows.h contains the functions that we want to use to read/write memory
#include <TlHelp32.h> // TlHelp32.h contains some functions useful when we want to get modules (dlls) of the target process
#include <string>
#include <cstring>
#include <vector>
#include <string.h>

class HookMethodInDll
{
public:
	//Gets a module by his name inside the mEntry
	MODULEENTRY32 GetModule(std::string searchedModuleName);

	void InstallHookOnMethod();

	HookMethodInDll(std::string targetProcessName, std::string targetDll, std::string targetMethodName);

	~HookMethodInDll();

private:
	bool AttachProcess();

	DWORD m_pid; // The ProcessID
	HANDLE m_hProcess; // The handle of process
	std::string m_targetProcessName;
	std::string m_targetMethodName;
	std::string m_targetDLLName;

	MODULEENTRY32 m_targetDll;
	DWORD m_targetDLL_Base;
};
#endif