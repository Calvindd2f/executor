#include "EHExceptionRecord.h"

std::string EHExceptionRecord::ToString() const
{
    std::stringstream ss;
    ss << "Timestamp: " << std::chrono::system_clock::to_time_t(Timestamp) << "\n"
       << "Message: " << Message << "\n"
       << "Source: " << Source << "\n"
       << "StackTrace: " << StackTrace << "\n"
       << "ExceptionCode: 0x" << std::hex << ExceptionCode << "\n"
       << "ExceptionAddress: " << ExceptionAddress << "\n";
    if (InnerException)
    {
        ss << "InnerException: " << InnerException->ToString();
    }
    return ss.str();
}