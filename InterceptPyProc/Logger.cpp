#include "Logger.h"


using namespace dbg;

Logger::Logger(LPCWSTR suspiciousFile) :
	suspiciousFile(suspiciousFile),
	appName(L"C:\\Users\\Latif\\AppData\\Local\\Programs\\Python\\Python39\\python.exe"),
	cwd(L"D:\\everything\\diploma\\PyDetect\\Logger")
{
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));
	startupInfo.cb = sizeof(startupInfo);
}

Logger::~Logger() {

}

void Logger::MergeFiles() {
	LPCWSTR constArgs = L"python.exe merge.py ";
	int argsLen = lstrlenW(constArgs);
	int sfLen = lstrlenW(suspiciousFile);
	int sumLenBytes = sizeof(WCHAR) * (argsLen + sfLen + 1);
	LPWSTR args = (LPWSTR)malloc(sumLenBytes);
	ZeroMemory(args, sumLenBytes);
	//lstrcpyW(args, constArgs);
	lstrcpynW(args, constArgs, argsLen + 1);
	lstrcpynW(args + argsLen, suspiciousFile, sfLen + 1);

	std::cout << "Creating DEBUG file" << std::endl;

	if (
		!CreateProcess(appName,
			args,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			cwd,
			&startupInfo,
			&procInfo)
		)
	{
		Debug::GetLastErrorAsString("Failed to Create Process (MergeFiles)");
	}

	WaitForSingleObject(procInfo.hProcess, INFINITE);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
}

void Logger::MakeLogs() {
	LPCWSTR constArgs = L"python.exe debug\\debug.py";
	int argsLen = lstrlenW(constArgs);
	int sumLenBytes = sizeof(WCHAR) * (argsLen + 1);
	LPWSTR args = (LPWSTR)malloc(sumLenBytes);
	ZeroMemory(args, sumLenBytes);
	lstrcpynW(args, constArgs, argsLen + 1);

	std::cout << "Creating LOGS" << std::endl;

	if (
		!CreateProcess(appName,
			args,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			cwd,
			&startupInfo,
			&procInfo)
		)
	{
		Debug::GetLastErrorAsString("Failed to Create Process (MakeLogs)");
	}

	WaitForSingleObject(procInfo.hProcess, INFINITE);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
}