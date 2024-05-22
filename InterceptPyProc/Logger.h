#pragma once
#include <Windows.h>
#include <iostream>
#include "Debug.h"

class Logger
{
public:
	Logger(LPCWSTR suspiciousFile);
	~Logger();
	void MergeFiles();
	void MakeLogs();
private:
	LPCWSTR suspiciousFile;
	LPCWSTR appName;
	LPCWSTR cwd;
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION procInfo;
};

