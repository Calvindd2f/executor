#include "EHExceptionRecord.h"

std::string EHExceptionRecord::ToString() const
{
    std::stringstream ss;

    ss << "Timestamp: " << (Timestamp.time_since_epoch().count() != 0 ? std::chrono::system_clock::to_time_t(Timestamp) : 0) << "\n"
       << "Message: " << (Message.empty() ? "N/A" : Message) << "\n"
       << "Source: " << (Source.empty() ? "N/A" : Source) << "\n"
       << "StackTrace: " << (StackTrace.empty() ? "N/A" : StackTrace) << "\n"
       << "ExceptionCode: "
       << "0x" << std::hex << ExceptionCode << std::dec << "\n"
       << "ExceptionAddress: " << (ExceptionAddress ? ExceptionAddress : reinterpret_cast<void *>(0)) << "\n";

    if (InnerException)
    {
        ss << "InnerException: " << InnerException->ToString();
    }

    return ss.str();
}
