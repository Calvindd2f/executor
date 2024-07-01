#pragma once

#include <string>

class StringUtil
{
public:
    static std::string ConvertWstringToUtf8String(const std::wstring &wstr);
    static std::wstring ConvertUtf8StringToWstring(const std::string &str);
    static std::string Trim(const std::string &str);
};