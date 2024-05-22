#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include "Debug.h"
#include "NativeStrutcts.h"
#include "StringParser.h"
#include "Logger.h"

using namespace dbg;

class Coordinator
{
private:
	// GET PROCESS DATA METHODS
	const std::string procName;
	std::vector<std::string> procList;
	bool IsRunAsAdmin();
	void ForkProcess(const std::string & procName, const int pid);
	int FindProcIdByName(const std::string & procName);

	void GainDebugPrivilege();
	void AttachToProcess(int pid);
	void GetHandle(int pid);
	void BreakProcess(int pid);
	void GetNativeFunction();

	_PROCESS_BASIC_INFORMATION* GetPBI(int pid);
	PPEB GetPEB(int pid, _PROCESS_BASIC_INFORMATION* pPBI);
	MY_PRTL_USER_PROCESS_PARAMETERS GetProcParams(int pid, PPEB pPebReadAccessed);
	PWSTR GetCLIArgs(int pid, MY_PRTL_USER_PROCESS_PARAMETERS procParams);
	PWSTR GetCWD(int pid, MY_PRTL_USER_PROCESS_PARAMETERS procParams);

	// ANALYZE METHODS
	
	void PrependPathToFiles(std::vector<std::wstring> & argList, std::wstring cwd);
	void ValidateFiles(std::vector<std::wstring> & filePathes);
	void RunLogger(std::wstring filePath);


	// FIELDES
	std::map<int, HANDLE> handlersById;
	pfnNtQueryInformationProcess NtQueryInformationProcess;

public:
	Coordinator(const std::vector<std::string>& procList);
	Coordinator(const std::string& procName);
	void MainLoop();
};

