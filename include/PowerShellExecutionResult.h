#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

public
ref class PowerShellExecutionResult
{
public:
    PowerShellExecutionResult();
    ~PowerShellExecutionResult();
    !PowerShellExecutionResult(); // Finalizer

    property bool Success
    {
        bool get() { return _success; }
        void set(bool value) { _success = value; }
    }

    property String ^ Output {
        String ^ get() { return _output; } void set(String ^ value) { _output = value; }
    }

        property String ^
        ErrorMessage {
            String ^ get() { return _errorMessage; } void set(String ^ value) { _errorMessage = value; }
        }

        // Method to get unmanaged version of the result
        void
        ToUnmanaged(PowerShellExecutionResultUnmanaged *result);

    // Method to populate from unmanaged version
    void FromUnmanaged(const PowerShellExecutionResultUnmanaged *result);

private:
    bool _success;
    String ^ _output;
    String ^ _errorMessage;
};

// Unmanaged structure for interop
[StructLayout(LayoutKind::Sequential)] struct PowerShellExecutionResultUnmanaged
{
    [MarshalAs(UnmanagedType::U1)] bool Success;

    [MarshalAs(UnmanagedType::LPStr)] char *Output;

    [MarshalAs(UnmanagedType::LPStr)] char *ErrorMessage;
};