#include "StringParser.h"
StringParser::StringParser(PWSTR pwStr, std::wstring delim) : inputStr(pwStr), m_Delim(delim) {}
StringParser::StringParser(PWSTR pwStr, wchar_t* delim) : inputStr(pwStr), m_Delim(delim) {}

void StringParser::Parse() {
    std::wstring tmpStr;
    bool delimFound = false;
    for (auto it = this->inputStr.begin(); it < this->inputStr.end(); it++) {
        auto X = *it;
        for (auto itDel = this->m_Delim.begin(); itDel < this->m_Delim.end(); itDel++) {
            if (*it != *itDel) {
                delimFound = false;
                break;
            }
            else
                delimFound = true;
        }
        if (delimFound) {
            if (tmpStr.size() > 0)
                this->parsedData.push_back(tmpStr);
            tmpStr = L"";
        }
        else {
            tmpStr.push_back(X);
        }
    }
    this->parsedData.push_back(tmpStr);
}

std::vector<std::wstring> StringParser::GetParsedData() {
    return this->parsedData;
}
