#pragma once
#include <string>
#include <vector>
#include <Windows.h>

class StringParser
{
private:
	std::wstring inputStr;
	std::wstring m_Delim;
	std::vector<std::wstring> parsedData;

public:
	StringParser(PWSTR pwStr, std::wstring delim);
	StringParser(PWSTR pwStr, wchar_t* delim);
	void Parse();
	std::vector<std::wstring> GetParsedData();
};
