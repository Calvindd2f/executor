#include "ProcessUtil.h"
#include <windows.h>
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

String ^ ProcessUtil::GetExecutableDirectoryPath()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    std::string path = std::string(buffer).substr(0, pos);
    return gcnew String(path.c_str());
}