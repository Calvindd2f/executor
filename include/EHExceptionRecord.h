#pragma once

#include <string>
#include <cstdint>
#include <chrono>
#include <sstream>

struct EHExceptionRecord
{
    // C# properties
    std::string Message;
    std::string StackTrace;
    std::string Source;
    std::chrono::system_clock::time_point Timestamp;
    EHExceptionRecord *InnerException = nullptr;

    // C++ specific members
    uint32_t ExceptionCode = 0;
    uint32_t ExceptionFlags = 0;
    EHExceptionRecord *ExceptionRecord = nullptr;
    void *ExceptionAddress = nullptr;
    uint32_t NumberParameters = 0;

    struct EHParameters
    {
        static const uint32_t MaxParameters = 15;
        uintptr_t Parameters[MaxParameters] = { 0 };
    } Params;

    // Constructor
    EHExceptionRecord() = default;

    // Constructor with exception details
    EHExceptionRecord(const std::string &message, const std::string &stackTrace, const std::string &source)
        : Message(message), StackTrace(stackTrace), Source(source) {}

    // Copy constructor
    EHExceptionRecord(const EHExceptionRecord &other)
    {
        Message = other.Message;
        StackTrace = other.StackTrace;
        Source = other.Source;
        Timestamp = other.Timestamp;
        InnerException = other.InnerException;
        ExceptionCode = other.ExceptionCode;
        ExceptionFlags = other.ExceptionFlags;
        ExceptionRecord = other.ExceptionRecord;
        ExceptionAddress = other.ExceptionAddress;
        NumberParameters = other.NumberParameters;
        for (auto i = 0; i < Params.MaxParameters; ++i)
            Params.Parameters[i] = other.Params.Parameters[i];
    }

    // Move constructor
    EHExceptionRecord(EHExceptionRecord &&other) noexcept
    {
        Message = std::move(other.Message);
        StackTrace = std::move(other.StackTrace);
        Source = std::move(other.Source);
        Timestamp = std::move(other.Timestamp);
        InnerException = std::move(other.InnerException);
        ExceptionCode = std::move(other.ExceptionCode);
        ExceptionFlags = std::move(other.ExceptionFlags);
        ExceptionRecord = std::move(other.ExceptionRecord);
        ExceptionAddress = std::move(other.ExceptionAddress);
        NumberParameters = std::move(other.NumberParameters);
        std::move(other.Params.Parameters, other.Params.Parameters + Params.MaxParameters, Params.Parameters);
    }

    // Destructor
    ~EHExceptionRecord() = default;

    // Copy assignment operator
    EHExceptionRecord &operator=(const EHExceptionRecord &other)
    {
        if (this != &other)
        {
            Message = other.Message;
            StackTrace = other.StackTrace;
            Source = other.Source;
            Timestamp = other.Timestamp;
            InnerException = other.InnerException;
            ExceptionCode = other.ExceptionCode;
            ExceptionFlags = other.ExceptionFlags;
            ExceptionRecord = other.ExceptionRecord;
            ExceptionAddress = other.ExceptionAddress;
            NumberParameters = other.NumberParameters;
            for (auto i = 0; i < Params.MaxParameters; ++i)
                Params.Parameters[i] = other.Params.Parameters[i];
        }
        return *this;
    }

    // Move assignment operator
    EHExceptionRecord &operator=(EHExceptionRecord &&other) noexcept
    {
        if (this != &other)
        {
            Message = std::move(other.Message);
            StackTrace = std::move(other.StackTrace);
            Source = std::move(other.Source);
            Timestamp = std::move(other.Timestamp);
            InnerException = std::move(other.InnerException);
            ExceptionCode = std::move(other.ExceptionCode);
            ExceptionFlags = std::move(other.ExceptionFlags);
            ExceptionRecord = std::move(other.ExceptionRecord);
            ExceptionAddress = std::move(other.ExceptionAddress);
            NumberParameters = std::move(other.NumberParameters);
            std::move(other.Params.Parameters, other.Params.Parameters + Params.MaxParameters, Params.Parameters);
        }
        return *this;
    }

    // Method to get a string representation of the exception record
    std::string ToString() const
    {
        std::ostringstream ss;
        ss << "Message: " << Message << std::endl
           << "StackTrace: " << StackTrace << std::endl
           << "Source: " << Source << std::endl
           << "Timestamp: " << std::chrono::system_clock::to_time_t(Timestamp) << std::endl;
        return ss.str();
    }
};

