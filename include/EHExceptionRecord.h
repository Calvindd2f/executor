#pragma once

#include <string>
#include <cstdint>
#include <chrono>

struct EHExceptionRecord
{
    // C# properties
    std::string Message;
    std::string StackTrace;
    std::string Source;
    std::chrono::system_clock::time_point Timestamp;
    EHExceptionRecord *InnerException;

    // C++ specific members
    uint32_t ExceptionCode;
    uint32_t ExceptionFlags;
    EHExceptionRecord *ExceptionRecord;
    void *ExceptionAddress;
    uint32_t NumberParameters;

    struct EHParameters
    {
        static const uint32_t MaxParameters = 15;
        uintptr_t Parameters[MaxParameters];
    } Params;

    // Constructor
    EHExceptionRecord();

    // Constructor with exception details
    EHExceptionRecord(const std::string &message, const std::string &stackTrace, const std::string &source);

    // Copy constructor
    EHExceptionRecord(const EHExceptionRecord &other);

    // Move constructor
    EHExceptionRecord(EHExceptionRecord &&other) noexcept;

    // Destructor
    ~EHExceptionRecord();

    // Copy assignment operator
    EHExceptionRecord &operator=(const EHExceptionRecord &other);

    // Move assignment operator
    EHExceptionRecord &operator=(EHExceptionRecord &&other) noexcept;

    // Method to get a string representation of the exception record
    std::string ToString() const;
};