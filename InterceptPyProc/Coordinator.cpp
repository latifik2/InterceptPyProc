#include "Coordinator.h"
using namespace dbg;

Coordinator::Coordinator(const std::vector<std::string> & procList) {}
Coordinator::Coordinator(const std::string & procName): procName(procName) {
	if (!IsRunAsAdmin()) {
		std::cout << "Permission denied! Exiting." << std::endl;
		exit(1);
	}
	std::cout << "Access granted!" << std::endl;

	NtQueryInformationProcess = NULL;
	GetNativeFunction();
	//Need some func to validate procName <procName>.exe
}

bool Coordinator::IsRunAsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&NtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0, &pSid)) {

		if (!CheckTokenMembership(NULL, pSid, &fIsRunAsAdmin)) {
			fIsRunAsAdmin = FALSE;
		}
		FreeSid(pSid);
	}
	return (fIsRunAsAdmin == TRUE);
}

void Coordinator::MainLoop() {
	int pid;
	while (true) {
		while (!(pid = FindProcIdByName(procName))) {}
		ForkProcess(procName, (const int)pid);
	}
}

int Coordinator::FindProcIdByName(const std::string& procName) {
	const char* procNameCStr = procName.c_str();

	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	int pid = 0;
	BOOL hResult;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cerr << "Error creating process snapshot" << std::endl;
		return 1;
	}

	pe.dwSize = sizeof(PROCESSENTRY32);

	hResult = Process32First(hSnapshot, &pe);

	_bstr_t tmp(pe.szExeFile);
	const char* fileName = tmp;

	while (hResult) {
		// if we find the process: return process ID
		if (strcmp(procNameCStr, fileName) == 0) {
			pid = pe.th32ProcessID;
			break;
		}
		hResult = Process32Next(hSnapshot, &pe);
		tmp = _bstr_t(pe.szExeFile);
		fileName = tmp;

	}

	return pid;
}

void Coordinator::ForkProcess(const std::string& procName, const int pid) {
	std::cout << std::format("[#] Process {} found! PID - {}", procName, pid) << std::endl;

	GainDebugPrivilege();
	AttachToProcess(pid);
	GetHandle(pid);
	BreakProcess(pid);
	_PROCESS_BASIC_INFORMATION* pPBI = GetPBI(pid);
	PPEB pPEB = GetPEB(pid, pPBI);
	MY_PRTL_USER_PROCESS_PARAMETERS procParams = GetProcParams(pid, pPEB);
	PWSTR commandLine = GetCLIArgs(pid, procParams);
	PWSTR cwd = GetCWD(pid, procParams);

	StringParser sp = StringParser(commandLine, L" ");
	sp.Parse();
	auto parsedData = sp.GetParsedData();

	PrependPathToFiles(parsedData, std::wstring(cwd));
	ValidateFiles(parsedData);
	RunLogger(parsedData[0]);

	 
}

void Coordinator::AttachToProcess(int pid) {
	if (DebugActiveProcess(pid) == 0) {
		Debug::GetLastErrorAsString("Failed to attach to a process");
		exit(1);
	}
}

void Coordinator::GainDebugPrivilege() {
	LUID luid;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		Debug::GetLastErrorAsString("LookupPrivilegeValue error:");
	}
}

void Coordinator::GetHandle(int pid) {
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	handlersById[pid] = processHandle;
}

void Coordinator::BreakProcess(int pid) {
	if (DebugBreakProcess(handlersById[pid]) == 0) {
		Debug::GetLastErrorAsString("Failed to breakpoint a process");
		exit(1);
	}
}

void Coordinator::GetNativeFunction() {
	HINSTANCE hInstanceLib = LoadLibraryA("ntdll.dll");
	if (hInstanceLib == NULL) {
		std::cout << "Failed to load dll!" << std::endl;
		exit(1);
	}

	NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hInstanceLib, "NtQueryInformationProcess");
	if (NtQueryInformationProcess == NULL) {
		std::cout << "Failed to load procedure!" << std::endl;
		exit(1);
	}
}

_PROCESS_BASIC_INFORMATION* Coordinator::GetPBI(int pid) {
	ULONG lenghtPeb;
	ULONG pbiBufSize = sizeof(_PROCESS_BASIC_INFORMATION);
	_PROCESS_BASIC_INFORMATION* pPBI = NULL;
	pPBI = (_PROCESS_BASIC_INFORMATION*)malloc(pbiBufSize);

	NtQueryInformationProcess(handlersById[pid], ProcessBasicInformation, pPBI, pbiBufSize, &lenghtPeb);
	if (pPBI == NULL) {
		std::cout << "Failed to query Process Basic Information" << std::endl;
		exit(1);
	}
	return pPBI;
}

PPEB Coordinator::GetPEB(int pid, _PROCESS_BASIC_INFORMATION *pPBI) {

	SIZE_T pebBufSize = sizeof(_PEB);
	SIZE_T bytesRead;
	PPEB pPebBaseAddress = pPBI->PebBaseAddress;
	PPEB pPebReadAccessed = (PPEB)malloc(pebBufSize);

	if (ReadProcessMemory(handlersById[pid], (LPCVOID)pPebBaseAddress, pPebReadAccessed, pebBufSize, &bytesRead) == 0) {
		std::cout << "Failed to read PEB" << std::endl;
		exit(1);
	}
	return pPebReadAccessed;
}

MY_PRTL_USER_PROCESS_PARAMETERS Coordinator::GetProcParams(int pid, PPEB pPebReadAccessed) {
	int processParametersLength = sizeof(MY_RTL_USER_PROCESS_PARAMETERS);
	MY_PRTL_USER_PROCESS_PARAMETERS  processParametersBase = (MY_PRTL_USER_PROCESS_PARAMETERS)pPebReadAccessed->ProcessParameters;
	MY_PRTL_USER_PROCESS_PARAMETERS  processParametersBuffer = (MY_PRTL_USER_PROCESS_PARAMETERS)malloc(processParametersLength);
	if (ReadProcessMemory(handlersById[pid], (LPCVOID)processParametersBase, processParametersBuffer, processParametersLength, NULL) == 0) {
		std::cout << "Failed to read Process Parametres" << std::endl;
		exit(1);
	}
	return processParametersBuffer;
}

PWSTR Coordinator::GetCLIArgs(int pid, MY_PRTL_USER_PROCESS_PARAMETERS procParams) {
	UNICODE_STRING pCommandLine = procParams->CommandLine;
	PWSTR commandLine = (PWSTR)malloc(pCommandLine.Length);
	std::fill(commandLine, commandLine + pCommandLine.Length, 0);

	if (ReadProcessMemory(handlersById[pid], (LPCVOID)pCommandLine.Buffer, commandLine, pCommandLine.Length, NULL) == 0) {
		std::cout << "Failed to read Command Line" << std::endl;
		exit(1);
	}
	return commandLine;
}

PWSTR Coordinator::GetCWD(int pid, MY_PRTL_USER_PROCESS_PARAMETERS procParams) {
	CURDIR currentDirectory = procParams->CurrentDirectory;
	PWSTR dosPath = (PWSTR)malloc(currentDirectory.DosPath.Length);
	std::fill(dosPath, dosPath + currentDirectory.DosPath.Length, 0);

	if (ReadProcessMemory(handlersById[pid], (LPCVOID)currentDirectory.DosPath.Buffer, dosPath, currentDirectory.DosPath.Length, NULL) == 0) {
		std::cout << "Failed to read Dos Path" << std::endl;
		exit(1);
	}
	return dosPath;
}

// ANALYZE METHODS
void Coordinator::PrependPathToFiles(std::vector<std::wstring>& argList, std::wstring cwd) {
	for (auto it = argList.begin(); it < argList.end(); it++) {
		(*it).insert(0, cwd);
	}
}

void Coordinator::ValidateFiles(std::vector<std::wstring>& filePathes) {
	for (auto it = filePathes.begin(); it < filePathes.end(); it++) {
		if (!std::filesystem::exists(*it))
			it = filePathes.erase(it);
	}
}void Coordinator::RunLogger(std::wstring filePath) {

	// !!! TODO 
	// Separate Logger Class which will Create Python Processes to merge and log scripts

	Logger logger = Logger(filePath.c_str());
	logger.MergeFiles();
	logger.MakeLogs();
}
